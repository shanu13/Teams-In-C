#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <unistd.h>
#include <netinet/in.h>

#include <errno.h>

#include "../serverHeaders/server_header.h"
#include "../serverHeaders/handler_header.h"
#include "../serverHeaders/hb_tree.h"

#define MAGIC 0xABCDDCBA

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))



client_conn_data_t* 
CreateNewClient(int clientfd)
{
    client_conn_data_t* client = (client_conn_data_t*)malloc(sizeof(client_conn_data_t));
    
    client->fd = clientfd;
    client->buff = NULL;
    client->buffer_used = 0;
    client->buffer_allocd = 0;
    client->offset = 0;
    client->client_state = INITIAL_STATE;

    return client;
}


void
NewUserHandler(int epfd, int server_socket, struct epoll_event* ev)
{
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
    int client_socket = accept(server_socket,(struct sockaddr *)&client_address, &client_addrlen);

    if (client_socket < 0) {
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            // no incoming connnections
            return ;
        }else{
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
    }

    int flags = fcntl(client_socket, F_GETFL, 0);
    fcntl(client_socket, F_SETFL, O_NONBLOCK|flags);

    client_conn_data_t *client = CreateNewClient(client_socket);

    ev->events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLERR;
    ev->data.ptr = client;

   if (epoll_ctl(epfd,EPOLL_CTL_ADD,client_socket,ev) == -1) {
        perror("epoll_ctl  : clientsocket" );
        return;
    }

   //int min_size = 14;

    //setsockopt(client->fd, SOL_SOCKET, SO_RCVLOWAT, &min_size, sizeof(min_size));
    printf("New connection arrived %d \n",client_socket);
    return;
}


void
freeClient(client_conn_data_t* client)
{
    free(client->ptr);
    free(client->buff);
    free(client->user);
    free(client);
    //printf("client free\n");
    return;
}

void
closeConn(int epfd, client_conn_data_t* client, struct epoll_event* ev)
{
     int clientfd = client->fd;
     printf("closing connection with client %d \n", clientfd);
     if ((epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd,ev)) < 0 ){
            perror("error in removing \n");
     }
     close(clientfd);
     freeClient(client);
     return;
}

void
encode_header(message_header_t* header, uint8_t* buff, uint32_t* buff_size)
{
    uint32_t offset = 0;
    *((uint64_t *)(buff + offset)) = htonll(header->magic);
    offset += sizeof(header->magic);

    *((uint16_t *)(buff + offset)) = htons(header->len);
    offset+=sizeof(header->len);

    *((uint16_t *)(buff+offset)) = htons(header->message_type);
    offset+=sizeof(header->message_type);

    *buff_size = offset;

    return;
}

void 
encode_message(server_message_t* s_msg, uint8_t* buff, uint32_t* buff_size)
{
    encode_header(&(s_msg->header), buff,buff_size);
    memcpy(buff+ *buff_size, s_msg->message,sizeof(s_msg->message));
    *buff_size += sizeof(s_msg->message);
   
    return;
}


void
decode_header(uint8_t* in_buffer, message_header_t* header, uint32_t* offset)
{
    header->magic = ntohll(*((uint64_t *)(in_buffer+ *offset)));
    *offset+= sizeof(header->magic);
    
    header->len = ntohs(*((uint16_t *)(in_buffer+ *offset)));
    *offset+= sizeof(header->len);

    header->message_type = ntohs(*((uint16_t *)(in_buffer+ *offset)));
    *offset+= sizeof(header->message_type);

    return;
 }

void
decode_message_auth(uint8_t* in_buffer, message_auth_t* auth_token, uint32_t* offset)
{ 
   // printf("decode_message_auth \n");
    auth_token->user_len = ntohs(*((uint16_t *)(in_buffer+ *offset)));
    *offset+= sizeof(auth_token->user_len);

    auth_token->user = (char *)malloc((auth_token->user_len)*sizeof(char));
    memcpy(auth_token->user,in_buffer+ *offset,(auth_token->user_len));
    *offset+= auth_token->user_len;

    auth_token->password_len = ntohs(*((uint16_t *)(in_buffer+ *offset)));
    *offset+= sizeof(auth_token->password_len);

    auth_token->password = (char *)malloc((auth_token->password_len)*sizeof(char));
    memcpy(auth_token->password,in_buffer+ *offset,(auth_token->password_len));
    *offset+= auth_token->password_len;

    return;

}


int
VerifyHeader(client_conn_data_t* client)
{
    message_auth_t *auth_token = (message_auth_t *)malloc(sizeof(message_auth_t));
    client->ptr = auth_token;
    
    decode_header(client->buff,&(auth_token->header),&(client->offset));

    if(((message_auth_t *)client->ptr)->header.magic != MAGIC){
        printf("magic : %lu\n",((message_auth_t *)client->ptr)->header.magic );
        free(client->ptr);
        client->client_state = ERROR_STATE;
                 return -1;
    }

    return 0;
}

int
AuthVerify(const char* user, const char* password)
{ 
    if(strcmp(user,"shantanu") == 0  && strcmp(password,"shantanu") == 0)
        return 0;
    else if(strcmp(user,"mridul") == 0 &&  strcmp(password,"mridul") == 0)
        return 0;
    else if(strcmp(user,"udit") == 0  && strcmp(password,"udit") == 0)
        return 0;

    return -1;
 
}

int 
Authenticate(client_conn_data_t* client, hb_tree_t* Tree)
{
    hb_tree_t* tree = Tree;
    decode_message_auth(client->buff, client->ptr, &(client->offset));
    message_auth_t *temp =(message_auth_t *)client->ptr;
   // printf("username : %s\n",temp->user);
   // printf("username len %u\n",temp->user_len);

   // printf("password len %u\n",temp->password_len);
   // printf("password : %s\n",temp->password);
    if(AuthVerify(((message_auth_t*)client->ptr)->user,((message_auth_t *)client->ptr)->password) < 0){
            free(((message_auth_t*)client->ptr)->user);
            free(((message_auth_t *)client->ptr)->password);
            return -1;
    }
    
    if (hb_tree_insert(tree, temp->user, client->fd)  == NULL) {
            free(((message_auth_t*)client->ptr)->user);
            free(((message_auth_t *)client->ptr)->password);
            return -1;
    }
    client->user = (char*)calloc(strlen(temp->user),sizeof(char));
    strcpy(client->user,temp->user);
    free(((message_auth_t*)client->ptr)->user);
    free(((message_auth_t *)client->ptr)->password);

    return 0;
}


int
Write(int fd, uint8_t *buff, uint32_t buff_size)
{
    size_t total_bytes_send = 0;

    printf("buff size : %u\n",buff_size);
    //printf("Write Started\n");
    while(total_bytes_send < buff_size){
        //printf("Writing\n");
        size_t bytes_sent = write(fd, buff+total_bytes_send, buff_size-total_bytes_send);
       // printf("bytes sent : %zu\n",bytes_sent);
        
        if(bytes_sent == -1) return -1;
        
        total_bytes_send += bytes_sent;
        //printf("total bytes send %zu\n",total_bytes_send);
    }

    //printf("Write Complete\n");

    return 0;
}

int SendMessage(char* message, int fd)
{
    server_message_t* s_msg = (server_message_t *)malloc(sizeof(server_message_t));
    s_msg->header.magic = MAGIC;
    s_msg->header.message_type = SERVER_INFO;
    s_msg->message = message;
    s_msg->header.len = sizeof(message_header_t) + strlen(message);

    uint8_t* buff = (uint8_t *)malloc(sizeof(server_message_t)*sizeof(uint8_t));
    uint32_t buff_size = 0;
    encode_message(s_msg,buff,&buff_size);
    if(Write(fd,buff,buff_size) < 0) return -1;
    printf("Send Message bytes : %u\n",s_msg->header.len);
    //printf("SendMessage :%s\n",s_msg->message);
    //free(s_msg->message);
    free(s_msg);
    free(buff);

    //printf("Send Message Succesfully\n");

    return 0;

}

void resetClient(client_conn_data_t *client){
   free(client->ptr);
   free(client->buff);
   free(client->user);

   client->ptr = NULL;
   client->buff = NULL;
   client->buffer_used = 0;
   client->offset = 0;
   client->buffer_allocd = 0;
   client->client_state = INITIAL_STATE;
   client->user = NULL;

   return  ;
}

void
refreshClient(client_conn_data_t* client)
{
    free(client->ptr);
    free(client->buff);

    client->ptr = NULL;
    client->buff = NULL;
    client->buffer_used = 0;
    client->offset = 0;
    client->buffer_allocd = 0;

    return;
}

int 
Header(client_conn_data_t* client)
{
    message_chat_t* msg_chat = (message_chat_t*)malloc(sizeof(message_chat_t));
    client->ptr = msg_chat;
    decode_header(client->buff, &msg_chat->header , &client->offset);


    if (((message_chat_t*)client->ptr)->header.magic != MAGIC) {
            free(client->ptr);
            return-1;
    }

    return 0;
}


message_chat_t*
decodeMessageChat(client_conn_data_t* client)
{
   message_chat_t* msg_chat = client->ptr;
   uint8_t* buff = client->buff;
   uint32_t offset = client->offset;

   msg_chat->to_user_len = ntohs(*(uint16_t*)(buff+offset));
   offset+= sizeof(msg_chat->to_user_len);

   msg_chat->to_user = (char *)calloc(msg_chat->to_user_len, sizeof(char));
   memcpy(msg_chat->to_user, buff+offset, msg_chat->to_user_len);
   offset += msg_chat->to_user_len;

   msg_chat->message_len = ntohs(*(uint16_t*)(buff+offset));
   offset+= sizeof(msg_chat->message_len);

   msg_chat->message = (char *)calloc(msg_chat->message_len, sizeof(char));
   memcpy(msg_chat->message, buff+offset, msg_chat->message_len);
   offset += msg_chat->message_len;

   client->offset = offset;
    
   return msg_chat;

}
