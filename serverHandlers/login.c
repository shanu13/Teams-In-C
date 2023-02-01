
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../serverHeaders/server_header.h"

int AUTH(char* id, char* password){
	if(strcmp(id,"a")==0 && strcmp(password,"a") == 0){
		return 1;
	}
	
	if(strcmp(id,"b")==0 && strcmp(password,"b")==0){
		return 1;
	}

	return 0;
}
