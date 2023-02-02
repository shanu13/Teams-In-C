#include <stdint.h>

#define SIZE_HEADER sizeof(message_header_t)

typedef enum message_types_e {
    MSG_TYPE_INVALID = 0,
    MSG_TYPE_AUTH = 1,
    MSG_TYPE_CHAT_MSG = 2,
    MSG_TYPE_MAX = 3,
} messsage_types_t;

 typedef enum states_e{
	ERROR_STATE =0,
	INITIAL_STATE =1,
    AUTH_STATE = 2,
	MESSAGE_STATE =3,
	CONN_CLOSE_STATE =4
}states_t;



typedef struct message_header_s {
    uint64_t                magic;
    uint16_t                len;
    uint16_t                message_type;
} message_header_t;

typedef struct message_auth_s {
    message_header_t        header;
    uint16_t                user_len;
    char                   *user;
    uint16_t                password_len;
    char                   *password;
} message_auth_t;

typedef struct message_chat_s {
    message_header_t        header;
    uint16_t                to_user_len;
    char                   *to_user;
    uint16_t                message_len;
    char                   *message;
} message_chat_t;

typedef struct server_message_s {
    message_header_t        header;
    char                   *message;
} server_message_t;


message_auth_t*             createAuthToken();

void                        encode_header(message_header_t header, uint8_t *out_buffer, uint32_t *out_buffer_used);

int                         encode_auth_msg(message_auth_t *auth_token , uint8_t *out_buffer, uint32_t *out_buffer_used);

int                         decode_header(message_header_t *header , uint8_t *buff, uint32_t  *buff_size);

server_message_t*           decode_server_message(uint8_t *buff);

void                        encodeMessage_send(message_chat_t* msg_chat, uint8_t* buff, uint32_t* buff_used);

void                        createMessage(uint8_t* buff, uint32_t* buff_used);

int                         sendMessage(int fd,uint8_t* buff, uint32_t buff_size);

message_chat_t*             decodeMessage_recv(uint8_t* buff);




