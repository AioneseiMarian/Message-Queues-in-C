#ifndef SERVER_H
#define SERVER_H

#include "../header/hash_table.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <json-c/json.h>

#define MAX_EVENTS 64
#define INITIALPUBQUEUECAP 4



typedef struct Client {
    struct sockaddr_in client_addr;
    int client_fd;
} Client;

typedef struct TaskQueue {
    int client_fd_queue[MAX_EVENTS];
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} TaskQueue;

typedef struct Server {
    int server_fd;
    int epoll_fd;
    struct sockaddr_in server_addr;
    HashTable* messages;
    HashTable* subscribtions;
    TaskQueue task_queue;
} Server;

extern pthread_mutex_t table_mutex;

void handle_Client_Read(Server* server, int client_fd);
void parse_received_json(Server* server, char* json_string, int client_fd);




#endif