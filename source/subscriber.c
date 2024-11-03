#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../header/message.h"

#define SERVER_PORT 8000
#define SERVER_IPADDR "127.0.0.2"

typedef struct{
    struct sockaddr_in server_addr;
    int server_fd;
}Client;

Client* client;

int init_client(){
    client = malloc(sizeof(Client));
 

 
    client->server_fd = socket(AF_INET, SOCK_STREAM, 0);
 
    if(client->server_fd < 0){
        printf("Unable to create socket\n");
        return -1;
    }
 
    printf("Socket created successfully\n");
 
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(SERVER_PORT);
    client->server_addr.sin_addr.s_addr = inet_addr(SERVER_IPADDR);
 
    if(connect(client->server_fd, (struct sockaddr*)&client->server_addr, sizeof(client->server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }

    return 0;
}

int main(void)
{
    if(init_client() == -1){
        perror("");
        exit(-1);
    }

    char server_message[2000], client_message[2000];
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    printf("Connected with server successfully\n");
 
    printf("Enter message: ");
    fgets(client_message, sizeof(client_message), stdin);
 
    if(send(client->server_fd, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }
 
    if(recv(client->server_fd, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
 
    printf("Server's response: %s\n",server_message);
 
    close(client->server_fd);
 
    return 0;
}