#include "../serverHeaders/server_header.h"

states_t AuthFail(){
	states_t var;
	var = ERROR_STATE;
	return var;
}

//states_t AuthFail(){
//	return ERROR_STATE;
//}
//
//states_t AuthSucc(){   
//	return MESSAGE_STATE;
//}
//
//states_t ConnClose(){
//	return CONN_CLOSE_STATE;
//}
