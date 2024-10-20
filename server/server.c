#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <message.h>


void* print_message(void* arg) {
    char* message = (char*) arg;
    printf("%s\n", message);
    pthread_exit(NULL);
}

typedef struct{
    struct sockaddr_in client_addr;
    int client_sock;
    int client_size;
}Connection;

typedef struct{
    struct sockaddr_in server_addr;
    int socket_desc;
}ServerSocket;

ServerSocket* serverSocket = NULL;

int init_server()
{
    serverSocket = calloc(sizeof(serverSocket), 0);
    serverSocket->socket_desc = socket(AF_INET, SOCK_STREAM, 0);
 
    if(serverSocket->socket_desc < 0){
        printf("Error while creating socket\n");
        return(-1);
    }
    printf("Socket created successfully\n");
 
    serverSocket->server_addr.sin_family = AF_INET;
    serverSocket->server_addr.sin_port = htons(2000);
    serverSocket->server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
    if(bind(serverSocket->socket_desc, (struct sockaddr*)&serverSocket->server_addr, sizeof(serverSocket->server_addr))<0){
        printf("Couldn't bind to the port\n");
        return(-1);
    }
    printf("Done with binding\n");
}

Connection *wait_connections(){
    Connection *connection = malloc(sizeof(Connection));
 
    if(listen(serverSocket->socket_desc, 1) < 0){
        printf("Error while listening\n");
        exit(-1);
    }
    printf("\nListening for incoming connections.....\n");
 
    connection->client_size = sizeof(connection->client_addr);
    connection->client_sock = accept(serverSocket->socket_desc, (struct sockaddr*)&connection->client_addr, &connection->client_size);
 
    if (connection->client_sock < 0){
        printf("Can't accept\n");
        exit(-1);
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(connection->client_addr.sin_addr), ntohs(connection->client_addr.sin_port));

    return connection;
}

void destroy_server(){
    close(serverSocket->socket_desc);
    free(serverSocket);
}

void destroy_connection(Connection* connection){
    close(connection->client_sock);
    free(connection);

}

int main() {
    init_server();

    Connection* connection = wait_connections(); 
    char server_message[2000], client_message[2000];
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    if (recv(connection->client_sock, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return -1;
    }
    printf("Msg from client: %s\n", client_message);
 
    strcpy(server_message, "This is the server's message.");
 
    if (send(connection->client_sock, server_message, strlen(server_message), 0) < 0){
        printf("Can't send\n");
        return -1;
    }
 
    pthread_t thread;
    char* msg = "Hello from the thread!";
    int ret = pthread_create(&thread, NULL, print_message, (void*) msg);
    if (ret) {
        fprintf(stderr, "Error creating thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_join(thread, NULL); 

    return 0;
}
