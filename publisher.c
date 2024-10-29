#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "message.h"

#define MSGSIZ 2048
 
typedef struct Server{
    char server_message[MSGSIZ];
    char client_message[MSGSIZ];
    int socket_desc;
    struct sockaddr_in server_addr;
}Server;

Server* initServer(char* _addr, int _port){
    Server* server = (Server*)malloc(sizeof(Server));
    if(server == NULL){
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    memset(server->server_message,'\0',sizeof(server->server_message));
    memset(server->client_message,'\0',sizeof(server->client_message));
    
    server->socket_desc = socket(AF_INET, SOCK_STREAM, 0);
 
    if(server->socket_desc < 0){
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_port = htons(_port);
    server->server_addr.sin_addr.s_addr = inet_addr(_addr);

    if(connect(server->socket_desc, (struct sockaddr*)&(server->server_addr), sizeof(server->server_addr)) < 0){
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }
    printf("Connected with server successfully\n");

    return server;
}

void sendMessage(Server* server){
    printf("Enter message: ");
    fgets(server->client_message, MSGSIZ, stdin);
 
    if(send(server->socket_desc, server->client_message, strlen(server->client_message), 0) < 0){
        perror("Unable to send message");
        exit(EXIT_FAILURE);
    }
     if(recv(server->socket_desc, server->server_message, sizeof(server->server_message), 0) < 0){
        perror("Error while receiving server's msg\n");
        exit(EXIT_FAILURE);
    }
    printf("Server's response: %s\n",server->server_message);
}

void closeServer(Server* server){
    close(server->socket_desc);
}

int main(void)
{
    Server* server = initServer("127.0.0.1", 8888);
 
    sendMessage(server);
    closeServer(server);
    
    return 0;
}