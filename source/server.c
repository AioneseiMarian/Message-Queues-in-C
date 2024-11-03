#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../header/queues.h"

#define INITIALPUBQUEUECAP 4


typedef struct{
    struct sockaddr_in client_addr;
    int client_fd;
    int client_size;
}Client;

typedef struct{
    int server_fd;
    struct sockaddr_in server_addr;
    //Messages heaps | Messages queues
    Heap **publisher_queues;
    RequestQueue  **req_queues;
}Server;

Heap** initPublisherQueues(){
    Heap** _queues = (Heap**)malloc(sizeof(Heap*) * MSG_TERMINAL);
    if(_queues == NULL){
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < MSG_TERMINAL ; ++i){
        _queues[i] = createQueue(INITIALPUBQUEUECAP);
    }
    return _queues;
}

RequestQueue** initRequestQueues(){
    RequestQueue** req_queue = (RequestQueue**) malloc(sizeof(RequestQueue*) * MSG_TERMINAL);
    if(req_queue == NULL){
        perror("Error allocating memory");
        exit(-1);
    }
    for(int i = 0; i < MSG_TERMINAL; i++){
        req_queue[i] = createReqQueue();
    }
    return req_queue;
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

    _server->publisher_queues = initPublisherQueues();
    _server->req_queues = initRequestQueues();

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
    case MSG_TERMINAL:
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
void printMessage(Message* _msg){
    printHeader(&_msg->header);
    write(STDOUT_FILENO, _msg->data, _msg->header.len);
    printf("\n");
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
void readMessage(Client* _publisher, Message* _msg){
    int totalRecv = 0;
        int bytesRead;
        while(totalRecv < _msg->header.len){
            bytesRead = recv(_publisher->client_fd, _msg->data + totalRecv, _msg->header.len - bytesRead, 0);
            if(bytesRead < 0){
                perror("Error receiving");
            }
            totalRecv += bytesRead;
        }   
}
void fetchPublications(Server* _server){
    Client* publisher = (Client*)malloc(sizeof(Client));
    if(publisher == NULL){
        perror("Error allocating publisher memory");
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
        msg->data = (char*)malloc(sizeof(char) * msg->header.len);
        if(msg == NULL){
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        readMessage(publisher, msg);
        //printMessage(msg);
        if(msg->header.type != MSG_TERMINAL){
            pushHeap(_server->publisher_queues[msg->header.type], *msg);
        }
    }while(msg->header.type != MSG_TERMINAL);
    close(publisher->client_fd);
    free(publisher);
}
void testMessageQueues(Server* _server){
    Message msg = popHeap(_server->publisher_queues[MSG_NOTIF]);
    printMessage(&msg);
}
void closeServer(Server* _server){
    close(_server->server_fd);
    free(_server->publisher_queues);
    free(_server);
}
int main() {
    setbuf(stdout, NULL);
    Server* server = initServer(SERVER_IPADDR, PUBLISHER_PORT); 
    fetchPublications(server);
    
    testMessageQueues(server);
    testMessageQueues(server);

    Server* server_sub = initServer(SERVER_IPADDR, SUBSCRIBER_PORT);

    closeServer(server);
    return 0;
}

