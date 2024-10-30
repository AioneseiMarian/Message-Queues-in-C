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
#define SERVER_IPADDR "127.0.0.1"

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

void* print_message(void* arg) {
    char* message = (char*) arg;
    printf("%s\n", message);
    pthread_exit(NULL);
}
void threading(){
    pthread_t thread;
    char* msg = "Hello from the thread!";
    int ret = pthread_create(&thread, NULL, print_message, (void*) msg);
    if (ret) {
        fprintf(stderr, "Error creating thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_join(thread, NULL); 
}

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

void listenClients(Server* _server, int _connections){
    if(listen(_server->server_fd, _connections) < 0){
        perror("Error while listening");
        exit(EXIT_FAILURE);
    }
}

void fetchMessages(Server* _server){
    Client* publisher = (Client*)malloc(sizeof(Client));
    if(publisher == NULL){
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    
    listenClients(_server, 1);
    printf("Listening publisher . . . \n");
    publisher->client_size = sizeof(publisher->client_addr);
    publisher->client_fd = accept(_server->server_fd, (struct sockaddr*)&(publisher->client_addr), &(publisher->client_size));
    if(publisher->client_fd < 0){
        perror("Can't accept publisher");
        exit(EXIT_FAILURE);
    }
    printf("Publisher connected at IP: %s and port: %i\n", inet_ntoa(publisher->client_addr.sin_addr), ntohs(publisher->client_addr.sin_port));

    int ret;
    Message* msg = (Message*)malloc(sizeof(Message));
    char header_buf[sizeof(MessageHeader)];
    do{
        if((ret = recv(publisher->client_fd, header_buf, sizeof(MessageHeader), 0)) < 0){
            printf("Error receiving message from publisher"); 
        }
        memcpy(&(msg->header), header_buf, sizeof(MessageHeader));
        // printf("type:%i\npriority:%i\npubID:%i\nlenght:%i\n",
        //         msg->header.type,
        //         msg->header.has_prioriry,
        //         msg->header.publisherID,
        //         msg->header.len);
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
    }while(msg->header.type != TERMINATE);
    
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
