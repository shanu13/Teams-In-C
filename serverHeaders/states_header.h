 typedef enum states_e{
	ERROR_STATE =0,
	INITIAL_STATE =1,
    AUTH_STATE = 2,
	MESSAGE_STATE =3,
	CONN_CLOSE_STATE =4
}states_t;


states_t AuthFail();
states_t AuthSucc();
states_t ConnClose();


typedef enum events_e {
	LOGIN_FAIL = 0,
	LOGIN_SUCC = 1,
	CHAT_MESSG = 2,
	CONN_CLOSE = 3
}events_t;


