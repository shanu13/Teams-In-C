#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
