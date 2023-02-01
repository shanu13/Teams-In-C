#include "states_header.h"
#include "hb_tree.h"
#include <stdint.h>

#define MAGIC 0xABCDDCBA
#define SIZE_HEADER sizeof(message_header_t)

//const uint64_t MAGIC = 0xABCDDCBA;


typedef enum message_types_e {
	MSG_TYPE_INVALID = 0,
	MSG_TYPE_AUTH = 1 ,
	MSG_TYPE_CHAT_MSG = 2 ,
	MSG_TYPE_MAX = 3,
    SERVER_INFO = 4
}message_types_t;


typedef struct message_header_s{
	uint64_t 		magic;
	uint16_t 		len;
	uint16_t 		message_type;
} message_header_t;


typedef struct message_auth_s{
	message_header_t		 header;
    uint16_t                 user_len;
	char 					 *user;
    uint16_t                 password_len;
	char 					 *password;
} message_auth_t;

typedef struct server_message_s{
    message_header_t         header;
    char                    *message;
} server_message_t;

typedef struct message_chat_s{
	message_header_t 		 header;
	char 					 *tosend;
	char 					 *message_to_send;
} message_chat_t;


typedef struct client_conn_data_s {
    void                     *ptr;
    int				          fd;
    uint8_t 		         *buff;
    uint32_t 			      buffer_used;
    uint32_t                  offset;
    uint32_t		          buffer_allocd;
    states_t                  client_state;
    char                     *user;
} client_conn_data_t;

typedef struct server_conn_data_s{
	int fd;
} server_conn_data_t;

