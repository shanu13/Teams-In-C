#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <unistd.h>

#include <errno.h>

#include "./serverHeaders/server_header.h"
//#include "./headers/states_header.h"
#include "./serverHeaders/handler_header.h"


#define MAX_EVENTS 10


int main() {
   struct epoll_event ev, events[MAX_EVENTS];
   struct sockaddr_in server_address;

   int server_socket;
   server_socket = socket(AF_INET,SOCK_STREAM,0);
   server_address.sin_family = AF_INET;
   server_address.sin_port = htons(9302);
   server_address.sin_addr.s_addr = INADDR_ANY;

   // making server socket non blocking

   int flags= fcntl(server_socket,F_GETFL,0);
   fcntl(server_socket,F_SETFL,flags | O_NONBLOCK);


   if(bind(server_socket,(struct sockaddr *)&server_address, sizeof(server_address))  < 0 ){
        perror("bind() failed : ");
        exit(EXIT_FAILURE);
   }

   if(listen(server_socket,10) < 0){
        perror("listen() failed");
        exit(EXIT_FAILURE);
   }

   server_conn_data_t  *server_data =(server_conn_data_t*) malloc(1*sizeof(server_conn_data_t));
   server_data->fd = server_socket;

   int epfd = epoll_create1(0);
   ev.events = EPOLLIN | EPOLLET|EPOLLOUT;
   ev.data.ptr = server_data;

  // printf("%d\n",((server_conn_data_t *)(ev.data.ptr))->fd);
     
   if(epoll_ctl(epfd,EPOLL_CTL_ADD,server_socket,&ev) == -1){
       perror("epoll ctl : server_socket ");
       exit(EXIT_FAILURE);
   }

    hb_tree_t* Tree = hb_tree_new();

    int nfds;
     
	while(1){

  		int nfds = epoll_wait(epfd,events,MAX_EVENTS,-1);
       // printf("server while loop %d\n", nfds);
		for(int n=0; n<nfds; n++){
         //   printf("server for loop with nfds %d\n", nfds);
			if(((server_conn_data_t *)events[n].data.ptr)->fd == server_socket){
              
                NewUserHandler(epfd,server_socket,&ev);
	          }
            else{
                    client_conn_data_t *cdp = ((client_conn_data_t *)events[n].data.ptr);
                    if (events[n].events & EPOLLERR) {
                        if (cdp->user != NULL) {
                            hb_tree_remove(Tree, cdp->user);
                        }
                        printf("EPOLLERR\n");
                        closeConn(epfd, cdp, &ev);
                        continue;
                    }

                    int clientfd =  cdp->fd;
                    
                   // printf("existing connection %d\n",clientfd);

                    uint8_t buff[1024];

                    memset(buff,0,sizeof(buff));
                    int ret = read(clientfd,buff,sizeof(buff));

                    printf("ret : %d\n",ret);
                   
                    if(ret == -1){
                        printf("client goes in blocking state\n");
                        continue;
                    }

                    if (ret == 0) { 
                        if (cdp->user != NULL) {
                            hb_tree_remove(Tree, cdp->user);
                        }
                        closeConn(epfd, cdp ,&ev);
                        continue;
                    }

                    if(cdp->buff == NULL){
                        cdp->buff = (uint8_t *)malloc(ret*sizeof(uint8_t));
                        if(cdp->buff == NULL){
                           printf("malloc null\n");
                        }
                        cdp->buffer_allocd = ret;
                    }else{
                        cdp->buff = realloc(cdp->buff,cdp->buffer_allocd+ret);
                        cdp->buffer_allocd += ret;
                    }
                    
                    memcpy(cdp->buff+cdp->buffer_used,buff,ret);
                    cdp->buffer_used += ret;

                    states_t curr_state =  cdp->client_state; 

                    //printf("current state %d\n",curr_state);
                    //printf("buffer used %u\n",cdp->buffer_used);

                    switch(curr_state){
                    
                        case INITIAL_STATE :
                           if(cdp->buffer_used >= SIZE_HEADER){
                               if(VerifyHeader(cdp) < 0){
                                   printf("Invalid Header \n");
                                    closeConn(epfd,cdp,&ev);
                                    continue;
                                }else{
                                    printf("INITIAl_STATE\n");
                                    cdp->client_state = AUTH_STATE;
                                }
                            }else{
                                continue;
                            }
                           printf("header len %u\n",((message_auth_t *)cdp->ptr)->header.len);
                               
                            

                         case AUTH_STATE :
                            if(cdp->buffer_used >= ((message_auth_t *)cdp->ptr)->header.len){ 
                                 printf("AUTH STATE\n");
                                 printf("cdp->buffer_used %u\n",cdp->buffer_used);
                                 if(Authenticate(cdp,Tree) < 0){
                                     SendMessage("401",cdp->fd);
                                     printf("Invalid Credentials\n");
                                     resetClient(cdp);
                                     continue; 
                                   //  closeConn(epfd,cdp,&ev);
                                 }else{ 
                                     printf("Successfully Login !!!\n");
                                     if(SendMessage("200",cdp->fd) < 0){
                                           perror("Cannot Send Message");
                                           break;
                                     }
                                     refreshClient(cdp);
                                     printf("clientfd = %d\n",cdp->fd);
                                     cdp->client_state = MESSAGE_STATE;
                                 }
                             }
                             break;

                        case MESSAGE_STATE : 
                             printf("Messsage State \n");
                             if (cdp->buffer_used >= SIZE_HEADER) {
                                 if(cdp->offset ==0 && Header(cdp) < 0){
                                   printf("Invalid Header \n");
                                    closeConn(epfd,cdp,&ev);
                                    continue;
                                }else{
                                   if (cdp->buffer_used >= ((message_auth_t *)cdp->ptr)->header.len) {
                                         message_chat_t *msg_chat = decodeMessageChat(cdp);
                                         //printf("to user : %s\n",msg_chat->to_user);
                                         //printf("message : %s\n",msg_chat->message);
                                         int sender_fd = hb_tree_search(Tree,msg_chat->to_user);
                                         uint32_t buff_size = 0;
                                         free(cdp->buff);
                                         cdp->buff = updateBuff(cdp->buff, &buff_size, msg_chat, cdp->user);
                                         Write(sender_fd,cdp->buff, buff_size);
                                         free(msg_chat->to_user);
                                         free(msg_chat->message);
                                         refreshClient(cdp);                                         
                                   }     
                                }
                             } 
                             break;
                    }  
               }
		}
	}

   close(server_socket);
   free(server_data);
   return 0;
 }
