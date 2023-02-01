#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <unistd.h>

#include <errno.h>

#include "header.h"


int main(){

    char cred[2][20];

    int server_socket = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_size = sizeof(client_address);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9302);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);


    bind(server_socket,(struct sockaddr *) &server_address, sizeof(server_address));

    listen(server_socket,5);

    int clientsocket;

    while( clientsocket = accept(server_socket,(struct sockaddr *)&client_address,&client_size)){

        for (int i=0; i<2; i++) {
            memset(cred[i],0,sizeof(cred[i]));
            int ret = read(clientsocket, cred[i], sizeof(cred[i]));
            if (ret < 0) {
                printf("Error in recieving message ret = %d error = %s\n", ret, strerror(errno));
            }
            printf("%s \n",cred[i]);
        }

        int response = Auth(cred[0],cred[1]);

        if(response == 0){
            printf("auth failed for this client %d \n",clientsocket);
            EOF;
            close(clientsocket);			
            continue;
        }

        char server_message[] = "Congratulations ! You reached the server";
        write(clientsocket, server_message, sizeof(server_message));

        close(clientsocket);
    }
    return 0;
}
