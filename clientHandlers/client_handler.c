#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#include <errno.h>
#include <arpa/inet.h>

#include "../clientHeaders/client_header.h"

#define MAGIC 0xABCDDCBA

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))


message_auth_t*
createAuthToken()
{
    message_auth_t *auth_token = (message_auth_t *)malloc(sizeof(message_auth_t));
    auth_token->header.magic = MAGIC;
    auth_token->header.message_type = MSG_TYPE_AUTH;
    auth_token->user  = NULL;
    auth_token->password = NULL;

    printf("Enter your username\n");
    scanf("%ms",&(auth_token->user));
    auth_token->user[strlen(auth_token->user)] = '\0';
    auth_token->user_len = strlen(auth_token->user);

    printf("Enter your password\n");
    scanf("%ms",&(auth_token->password));
    auth_token->password[strlen(auth_token->password)] = '\0';
    auth_token->password_len = strlen(auth_token->password);

    auth_token->header.len = sizeof(message_header_t)+auth_token->user_len + auth_token->password_len;

    return auth_token;
}


void
encode_header(message_header_t header, uint8_t *out_buffer, uint32_t *out_buffer_used)
{
    *((uint64_t *)(out_buffer)) = htonll(header.magic);
    *out_buffer_used+= sizeof(header.magic);

    *((uint16_t *)(out_buffer+ *out_buffer_used)) = htons(header.len);
    *out_buffer_used+= sizeof(header.len);

    *((uint16_t *)(out_buffer+ *out_buffer_used)) = htons(header.message_type);
    *out_buffer_used+= sizeof(header.message_type);
    
    return;
}

int
encode_auth_msg(message_auth_t *auth_token , uint8_t *out_buffer, uint32_t *out_buffer_used)
{
    encode_header(auth_token->header, out_buffer, out_buffer_used);
    
    *((uint16_t *)(out_buffer+ *out_buffer_used)) = htons(auth_token->user_len);
    *out_buffer_used += sizeof(auth_token->user_len);

    memcpy(out_buffer + *out_buffer_used, auth_token->user,auth_token->user_len);
    *out_buffer_used += auth_token->user_len;

    *((uint16_t *)(out_buffer+ *out_buffer_used)) = htons(auth_token->password_len);
    *out_buffer_used += sizeof(auth_token->password_len);

    memcpy(out_buffer + *out_buffer_used,auth_token->password,auth_token->password_len);
    *out_buffer_used += auth_token->password_len;

    return 0;

}
    
int
decode_header (message_header_t *header , uint8_t *buff, uint32_t *offset)
{
    header->magic = ntohll(*(uint64_t *)(buff+ *offset));
    *offset+= sizeof(header->magic);

    header->len = ntohs(*(uint16_t *)(buff+ *offset));
    *offset += sizeof(header->len);

    header->message_type = ntohs(*(uint16_t *)(buff+ *offset));
    *offset += sizeof(header->message_type);

    return 1;

}

server_message_t*
decode_server_message (uint8_t *buff)
{   
    //printf("decoding server message \n");
    server_message_t *s_msg = (server_message_t *)malloc(sizeof(server_message_t));
    s_msg->message = NULL;

    uint32_t offset =0;
    decode_header(&s_msg->header, buff, &offset);
    if (s_msg->header.magic != MAGIC) {
        free(s_msg);
        return NULL;
    }

   // printf("header decoded \n");
    printf("total length : %u\n",s_msg->header.len);
    printf("offset : %u\n",offset);
    //s_msg->message = (char *)malloc((s_msg->header.len - sizeof(message_header_t))*(sizeof(char)));
    s_msg->message = (char *)calloc((s_msg->header.len - sizeof(message_header_t)), sizeof(char));
    memcpy(s_msg->message, buff+offset, s_msg->header.len - sizeof(message_header_t));
    offset+= s_msg->header.len - sizeof(message_header_t);

    return s_msg;
}

void
encodeMessage_send(message_chat_t* msg_chat, uint8_t* buff, uint32_t* buff_used)
{
    encode_header(msg_chat->header, buff, buff_used);
    *((uint16_t* )(buff+*buff_used)) = ntohs(msg_chat->to_user_len);
    *buff_used += sizeof(msg_chat->to_user_len);

    memcpy(buff+*buff_used, msg_chat->to_user, msg_chat->to_user_len);
    *buff_used += msg_chat->to_user_len;

    *((uint16_t* )(buff+*buff_used)) = ntohs(msg_chat->message_len);
    *buff_used += sizeof(msg_chat->message_len);

     memcpy(buff+*buff_used, msg_chat->message, msg_chat->message_len);
    *buff_used += msg_chat->message_len;
}


void
createMessage(uint8_t* buff, uint32_t* buff_used)
{
    message_chat_t* msg_chat = (message_chat_t*)malloc(sizeof(message_chat_t));
    msg_chat->header.magic = MAGIC;
    msg_chat->header.message_type = MSG_TYPE_CHAT_MSG;

    printf("Enter username\n");
    scanf("%ms",&msg_chat->to_user);
    msg_chat->to_user_len = strlen(msg_chat->to_user);

    uint16_t len =0;
    size_t size =0;
    printf("Enter Message \n");
    len =  getline(&msg_chat->message, &size, stdin);
    msg_chat->message_len = len;

    msg_chat->header.len = sizeof(message_header_t) + msg_chat->to_user_len + msg_chat->message_len;

    buff = (uint8_t *)malloc(msg_chat->header.len * sizeof(char));

    encodeMessage_send(msg_chat, buff, buff_used);

    free(msg_chat->to_user);
    free(msg_chat->message);
    free(msg_chat);
   
    return;   
}

int
sendMessage(int fd, uint8_t* buff, uint32_t buff_size)
{
     size_t total_bytes_send = 0;

    //printf("buff size : %u\n",buff_size);
    //printf("Write Started\n");
    while(total_bytes_send < buff_size){
          size_t bytes_sent = write(fd, buff+total_bytes_send, buff_size-total_bytes_send);
       // printf("bytes sent : %zu\n",bytes_sent);
        if(bytes_sent == -1) return -1;
        
        total_bytes_send += bytes_sent;
        //printf("total bytes send %zu\n",total_bytes_send);
    }

    free(buff);
    //printf("Write Complete\n");

    return 0;

}

message_chat_t*
decodeMessage_recv(uint8_t* buff)
{
    message_chat_t* msg_chat = (message_chat_t*)malloc(sizeof(message_chat_t));
uint32_t offset =0;

    decode_header(&msg_chat->header, buff, &offset);
    if (msg_chat->header.magic != MAGIC) {
            free(msg_chat);
            return NULL;
    }

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
    
   return msg_chat;
}
