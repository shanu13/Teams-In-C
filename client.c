#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <unistd.h>

#include <errno.h>

#include "clientHeaders/client_header.h"

#define MAGIC 0xABCDDCBA

int main(int argc, char const *argv[])
{
    int client_socket; 
    client_socket = socket(AF_INET,SOCK_STREAM,0);
   
    struct sockaddr_in  server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9302);
    server_address.sin_addr.s_addr = INADDR_ANY;

	int connection_status = connect(client_socket,(struct sockaddr *) &server_address,sizeof(server_address));
	
	printf("%d\n",client_socket);
	
	if(connection_status == -1 ){
		printf("There is an error for making connection\n\n");
            close(client_socket);                       
			return 0;
	}
	else
        printf("connection established\n"); 

    //int min_size = 14;

    //setsockopt(client_socket,SOL_SOCKET,SO_SNDLOWAT,&min_size,sizeof(min_size));

    states_t curr_state = INITIAL_STATE;

    
    while(1){
    
        
        switch(curr_state){
    
            case INITIAL_STATE :{ 
                 message_auth_t *auth_token = createAuthToken();
                 uint8_t *out_buffer = (uint8_t *)malloc(sizeof(message_auth_t)*sizeof(uint8_t));
                 int out_buffer_used = 0;
                 encode_auth_msg(auth_token, out_buffer, &out_buffer_used);

                 size_t total_bytes_send =0;

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
                    printf("WRONG ID OR PASSWORD\n");
                  }
                 free(read_buff);
                 free(s_msg->message);
                 free(s_msg);

                  break;

               }

            case MESSAGE_STATE : 
                   continue;
                  //printf("Message_state\n");



        }


    } 

	close(client_socket);
	
   return 0;
 }
