#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <unistd.h>

#include <errno.h>

#include "clientHeaders/client_header.h"

#define MAGIC 0xABCDDCBA


states_t curr_state;


int main(int argc, char const *argv[])
{
    int client_socket = startNewConnection(); 
   // client_socket = socket(AF_INET,SOCK_STREAM,0);

   // //int flags = fcntl(client_socket, F_GETFL, 0);
   // //fcntl(client_socket, F_SETFL, flags|O_NONBLOCK);
   //
   // struct sockaddr_in  server_address;
   // server_address.sin_family = AF_INET;
   // server_address.sin_port = htons(9302);
   // server_address.sin_addr.s_addr = INADDR_ANY;

   // int connection_status = connect(client_socket,(struct sockaddr *) &server_address,sizeof(server_address));
   // 
   // printf("%d\n",client_socket);
   // 
   // if(connection_status == -1 ){
   // 	printf("There is an error for making connection\n\n");
   //         close(client_socket);                       
   // 		return 0;
   // }
   // else
   //     printf("connection established\n"); 

   // //int min_size = 14;

   // //setsockopt(client_socket,SOL_SOCKET,SO_SNDLOWAT,&min_size,sizeof(min_size));

    //states_t curr_state = INITIAL_STATE;

    
    while(1){
    
        
        switch(curr_state){
    
            case INITIAL_STATE :{ 
                 message_auth_t *auth_token = createAuthToken();
                 uint8_t *out_buffer = (uint8_t *)malloc(sizeof(message_auth_t)*sizeof(uint8_t));
                 uint32_t out_buffer_used = 0;
                 encode_auth_msg(auth_token, out_buffer, &out_buffer_used);

                 size_t total_bytes_send =0;
                 printf("initial out buffer used %u \n",out_buffer_used); 

                 while(total_bytes_send < out_buffer_used){
                     size_t bytes_send = write(client_socket,out_buffer+total_bytes_send, out_buffer_used-total_bytes_send);
                     printf("bytes send %zu \n",bytes_send);
                     if(bytes_send == -1){
                        perror("failed to send");
                     }

                    total_bytes_send +=bytes_send;
                }
                
                 free(auth_token->user);
                 free(auth_token->password);
                 free(auth_token);
                 curr_state = AUTH_STATE;
                 printf("INITIAL STATE DONE \n");
                 break;

              }
    
            case AUTH_STATE :{
                 //printf("client auth state\n");
                 uint8_t temp_buff[1024];
                 bzero(temp_buff,0);
                 uint8_t *read_buff = NULL;
                 uint32_t read_buff_size = 0;
                 size_t bytes_read = 0;
                 while((bytes_read = read(client_socket,temp_buff,sizeof(temp_buff))) > 0){
                     //printf("bytes read %zu\n",bytes_read);  

                     if(bytes_read == 0) break;

                    if(read_buff == NULL){
                        read_buff = (uint8_t *)malloc(bytes_read);
                        if(read_buff == NULL){
                            printf("Malloc error\n");
                        }
                    }else{
                        read_buff = realloc(read_buff,read_buff_size+bytes_read);
                    }
                    
                    memcpy(read_buff+read_buff_size,temp_buff,bytes_read);
                    read_buff_size += bytes_read;
                    //printf("%u\n",read_buff_size);

                    bool is_read_full = false;

                    if (read_buff_size >= SIZE_HEADER) {
                        uint16_t len = ntohs(*((uint16_t *)(read_buff+8)));
                        if (read_buff_size >= len ) {
                                is_read_full = true;
                        }
                    }

                    if (is_read_full){
                            break;
                    }

                 }

                 //if(read_buff_size == 0) continue;

                 printf("size of readbuff  %u\n",read_buff_size);

                 server_message_t * s_msg = decode_server_message(read_buff);
                 printf("server message : %s\n",s_msg->message);

                 if(s_msg != NULL && strcmp(s_msg->message, "200") == 0){
                   curr_state = MESSAGE_STATE;
                   printf("SUCCESSFULLY LOGIN !!\n");
                                      
                 }else{
                    curr_state = INITIAL_STATE;
                    printf("WRONG ID OR PASSWORD\n\n\n");
                  }
                 free(read_buff);
                 free(s_msg->message);
                 free(s_msg);

                  break;

               }

            case MESSAGE_STATE : {
                 //printf("Message_state\n");
                 int option;
                 printf("\n\n");
                 printf("Enter 1 to Send Message\nEnter 2 to Recieve Message\nEnter 3 to Logout\n");
                 scanf("%d",&option);

                 switch (option)  {
                    case 1 : {
                           uint8_t *buff = NULL;
                           uint32_t buff_used = 0;
                           
                           buff = createMessage(buff, &buff_used); 

                           if (sendMessage(client_socket, buff, buff_used) < 0) { 
                                 printf("Error in Sending Message \n");

                                 continue;
                            }else{
                                printf("Message send succesfully\n");
                            }

                           free(buff);
                            break;
                       }

                    case 2 : {
                           uint8_t temp_buff[1024];
                           bzero(temp_buff,0);
                           uint8_t *read_buff = NULL;
                           uint32_t read_buff_size = 0;
                           size_t bytes_read = 0;
                           while((bytes_read = read(client_socket,temp_buff,sizeof(temp_buff))) > 0){
                                 printf("bytes read %zu\n",bytes_read);  

                              if(bytes_read == 0) break;

                              if(read_buff == NULL){
                                  read_buff = (uint8_t *)malloc(bytes_read);
                                 if(read_buff == NULL){
                                       printf("Malloc error\n");
                                 }
                              }else{
                                  read_buff = realloc(read_buff,read_buff_size+bytes_read);
                              } 
                    
                              memcpy(read_buff+read_buff_size,temp_buff,bytes_read);
                              read_buff_size += bytes_read;
                              //printf("%u\n",read_buff_size);

                              bool is_read_full = false;

                              if (read_buff_size >= SIZE_HEADER) {
                                 uint16_t len = ntohs(*((uint16_t *)(read_buff+8)));
                                 if (read_buff_size >= len ) {
                                        is_read_full = true;
                                }  
                             }

                            if (is_read_full){
                                    break;
                            }

                         }  
                        
                        printf("read_buff %u\n",read_buff_size);
                        message_chat_t* msg_chat = decodeMessage_recv(read_buff);
                        printf("------------------------------------------------------------\n\n");
                        printf("Message from %s : %s\n\n",msg_chat->to_user, msg_chat->message);
                        printf("------------------------------------------------------------\n\n");

                        free(msg_chat->to_user);
                        free(msg_chat->message);
                        free(msg_chat);

                        break;
                      }

                  case 3 : {
                        
                        // printf("Logout\n");
                        // size_t bytes_sent = write(client_socket, NULL, 0);
                        // if (bytes_sent  == -1) {
                        //    perror("Error in Logout");
                        // } else {
                        //    printf("\n----------------------------------------------------------\n\n\n");
                        //    printf("Successfully Logout !!!\n\n\n");
                        //    printf("----------------------------------------------------------\n\n\n");

                        // }
                            printf("\n----------------------------------------------------------\n\n\n");
                            printf("Successfully Logout !!!\n\n\n");
                            printf("----------------------------------------------------------\n\n\n");
                             
                            close(client_socket);
                            sleep(2);

                            //curr_state = INITIAL_STATE;
                            client_socket = startNewConnection();
                          break;
                      }

                    default : printf("Wrong Option Selected\n");         
                  } 
    
             }                     

        }


    } 

	close(client_socket);
	
   return 0;
 }
