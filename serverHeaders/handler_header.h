
void                    NewUserHandler(int epfd, int server_socket, struct epoll_event *ev);

void                    closeConn(int epfd,client_conn_data_t * client,struct epoll_event *ev );

void                    freeClient(client_conn_data_t *client);

void                    decode_header(uint8_t *in_buffer, message_header_t *header, uint32_t *offset);

void                    decode_message_auth(uint8_t *in_buffer, message_auth_t *auth_token, uint32_t *offset);

int                     VerifyHeader(client_conn_data_t *client);

int                     AuthVerify(const char *user, const char *password);

int                     Authenticate(client_conn_data_t *client, hb_tree_t* Tree);

int                     Write(int fd, uint8_t *buff, uint32_t buff_sized);

int                     SendMessage(char *message, int fd);

void                    resetClient(client_conn_data_t *client);

void                    refreshClient(client_conn_data_t* client);

int                     Header(client_conn_data_t* client);

message_chat_t*         decodeMessageChat(client_conn_data_t* client);




