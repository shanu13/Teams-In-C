
typedef enum message_types_e {
    MSG_TYPE_INVALID = 0,
    MSG_TYPE_AUTH = 1,
    MSG_TYPE_CHAT_MSG = 2,
    MSG_TYPE_MAX = 3,
} messsage_types_t;

x=htons(35)

35 == ntohs(x)

htons (16)
htonl (32)

ntohs (16)

typedef struct message_header_s {
    uint64_t              magic;     // 0xabcddcba
    uint16_t              len;
	uint16_t              message_type;
} message_header_t;

##  make sure (sizeof(message_header_t) == 12)


typedef struct message_auth_s {
    message_header_t      header;
    char                 *user;
    char                 *pass;
} message_auth_t;

typedef struct message_chat_s {
    message_header_t      header;
    char                 *to_user;
    char                 *msg_to_send;
} message_chat_t;


message_chat_t* alloc_chat_msg (char *to_user, char *msg_to_send)
{
    message_chat_t  *obj = malloc(sizeof(message_chat_t));
    obj->header.magic = 0xabcddcba;
    obj->len          = sizeof(message_header_t) + strlen(to_user) + strlen(msg_to_send) + 2;
    obj->message_type = MSG_TYPE_CHAT_MSG;
    obj->to_user = strdup(to_user);
    obj->msg_to_send = strdup(msg_to_send);
    return obj;
}

void free_chat_msg (message_chat_t *obj)
{
    free(obj->to_user);
    free(obj->msg_to_send);
    free(obj);
}

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))


int  // 0 success -1 failure
encode_chat_msg (message_chat_t  *obj,
                 uint8_t         *out_buffer,
                 uint32_t         out_buffer_len,
                 uint32_t        *out_buffer_used)
{
    *((uint64_t *)out_buffer) = htonll(obj->header.magic);
    (*out_buffer_used) += sizeof(obj->header.magic);

    *((uint16_t *)(out_buffer + *out_buffer_used)) = htons(obj->header.len);
    (*out_buffer_used) += sizeof(obj->header.len);


    *((uint16_t *)(out_buffer + *out_buffer_used)) = htons(obj->header.message_type);
    (*out_buffer_used) += sizeof(obj->header.message_type);

}


typedef struct client_conn_data_s {
    int                   fd;
    uint8_t              *buff;
    uint32_t              buff_used;
    uint32_t              buff_allocd;
} client_conn_data_t;


8             2       2      user(7)      password(7)
<0xabcddcba>  <26>     <1>    <mitesh\0>   <mitesh\0>


buff[1024];
int ret = read(fd, buff, sizeof(buff));    return possibilities    -1 error
                                                          0 connection closed <TCP_FIN>
                                                          positive values <1 to sizeof(buff)>
ret == 2;

client_conn_data_t *cdp = .....;

if (cdp->buff == NULL) {
   cdp->buff = malloc(ret);
   cdp->buff_allocd = ret;
   cdp->buff_used = 0;
} else {
   // cdp->buff := 0x7ff0004
   cdp->buff = realloc(cdp->buff, cdp->buff_allocd + ret);
   // cdp->buff := 0x7f80002
   cdp->buff_allocd = cdp->buff_allocd + ret;
}

memcpy(cdp->buff + cdp->buff_used, buff, ret);
cdp->buff_used += ret;



