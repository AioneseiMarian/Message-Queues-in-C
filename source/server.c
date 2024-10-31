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
    struct sockaddr_in client_addr;
    int client_fd;
    int client_size;
}Client;

typedef struct{
    int server_fd;
    struct sockaddr_in server_addr;
    //Messages heaps | Messages queues
}Server;

Server* initServer(char* _addr, int _port){
    Server* _server = (Server*)malloc(sizeof(Server));
    if(_server == NULL){
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    _server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_server->server_fd < 0){ 
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created succesfully\n");

    _server->server_addr.sin_family = AF_INET;
    _server->server_addr.sin_port = htons(_port);
    _server->server_addr.sin_addr.s_addr = inet_addr(_addr);

    if(bind(_server->server_fd, (struct sockaddr*)&(_server->server_addr), sizeof(_server->server_addr)) < 0){
        perror("Error on binding");
        exit(EXIT_FAILURE);
    }
    return _server;
}
char* getStringType(MsgType _type){
    switch(_type){
    case MSG_BIN_DATA:
        return "Binary Data";
    case MSG_NOTIF:
        return "Notification";
    case MSG_SYS_INFO:
        return "System information";
    case MSG_TASK:
        return "Task";
    case TERMINAL:
        return "Termination message";
    }
    return NULL;
}
void printHeader(MessageHeader* _hdr){
    printf("type: %s\npriority: %s\npubID: %i\nlenght: %i\n",
                getStringType(_hdr->type),
                _hdr->has_prioriry ? "High" : "Low",
                _hdr->publisherID,
                _hdr->len);
}
void listenClient(Server* _server, Client* _client){
    if(listen(_server->server_fd, 1) < 0){
        perror("Error while listening");
        exit(EXIT_FAILURE);
    }
    printf("Listening client . . . \n");
    _client->client_size = sizeof(_client->client_addr);
    _client->client_fd = accept(_server->server_fd, (struct sockaddr*)&(_client->client_addr), &(_client->client_size));
    if(_client->client_fd < 0){
        perror("Can't accept publisher");
        exit(EXIT_FAILURE);
    }
    printf("Publisher connected at IP: %s and port: %i\n",
            inet_ntoa(_client->client_addr.sin_addr),
            ntohs(_client->client_addr.sin_port));

}

void fetchMessages(Server* _server){
    Client* publisher = (Client*)malloc(sizeof(Client));
    if(publisher == NULL){
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    listenClient(_server, publisher);
    int ret;
    Message* msg = (Message*)malloc(sizeof(Message));
    char header_buf[sizeof(MessageHeader)];
    do{
        if((ret = recv(publisher->client_fd, header_buf, sizeof(MessageHeader), 0)) < 0){
            printf("Error receiving message from publisher"); 
        }
        memcpy(&(msg->header), header_buf, sizeof(MessageHeader));
        printHeader(&msg->header);
        msg->data = (char*)malloc(sizeof(char) * msg->header.len);
        if(msg == NULL){
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        int totalRecv = 0;
        int bytesRead;
        while(totalRecv < msg->header.len){
            bytesRead = recv(publisher->client_fd, msg->data + bytesRead, msg->header.len - bytesRead, 0);
            if(bytesRead < 0){
                perror("Error receiving");
            }
            totalRecv += bytesRead;
        }
        int tmpfd = open("tmp.txt", O_CREAT | O_WRONLY | O_APPEND, 0664);
        write(tmpfd, msg->data, msg->header.len);
        printf("%s\n", msg->data);
    }while(msg->header.type != TERMINAL);
    
    close(publisher->client_fd);
    free(publisher);
}

void closeServer(Server* _server){
    close(_server->server_fd);
    free(_server);
}

int main() {
    setbuf(stdout, NULL);
    Server* server = initServer(SERVER_IPADDR, SERVER_PORT); 
    fetchMessages(server);
    closeServer(server);

    return 0;
}
