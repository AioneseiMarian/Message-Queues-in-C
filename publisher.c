#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "message.h"

#define IPADDR "127.0.0.1"
#define PORT 8000
#define MSGSIZ 2048
#define MSGFILENAME "publ_db.txt"
#define TEMPFILENAME "publ_db2.txt"

typedef struct Publisher{
    char server_message[MSGSIZ];
    char client_message[MSGSIZ];
    int socket_desc;
    struct sockaddr_in server_addr;
    MessageHeader msg_header;
    int db_fd;
}Publisher;

Publisher* initServer(char* _addr, int _port){
    Publisher* _pub = (Publisher*)malloc(sizeof(Publisher));
    if(_pub == NULL){
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    memset(_pub->server_message,'\0',sizeof(_pub->server_message));
    memset(_pub->client_message,'\0',sizeof(_pub->client_message));
    
    _pub->socket_desc = socket(AF_INET, SOCK_STREAM, 0);
 
    if(_pub->socket_desc < 0){
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    _pub->server_addr.sin_family = AF_INET;
    _pub->server_addr.sin_port = htons(_port);
    _pub->server_addr.sin_addr.s_addr = inet_addr(_addr);

    _pub->db_fd = open(MSGFILENAME, O_RDONLY);
    if(_pub->db_fd == -1){
        perror("Error opening messages file");
        exit(EXIT_FAILURE);
    }

    if(connect(_pub->socket_desc, (struct sockaddr*)&(_pub->server_addr), sizeof(_pub->server_addr)) < 0){
        perror("Unable to connect");
        //exit(EXIT_FAILURE);
    }
    printf("Connected with server successfully\n");

    return _pub;
}

void sendMessage(Publisher* _pub){
    printf("Enter message: ");
    fgets(_pub->client_message, MSGSIZ, stdin);
 
    if(send(_pub->socket_desc, _pub->client_message, strlen(_pub->client_message), 0) < 0){
        perror("Unable to send message");
        exit(EXIT_FAILURE);
    }
     if(recv(_pub->socket_desc, _pub->server_message, sizeof(_pub->server_message), 0) < 0){
        perror("Error while receiving server's msg\n");
        exit(EXIT_FAILURE);
    }
    printf("Server's response: %s\n",_pub->server_message);
}

void fetchMessages(Publisher* _pub){
    setbuf(stdout, NULL);
    if(lseek(_pub->db_fd, 0, SEEK_END) == 0){
        printf("Empty database");
        exit(EXIT_FAILURE);
    }
    lseek(_pub->db_fd, 0, SEEK_SET);
    int temp_fd = open(TEMPFILENAME, O_CREAT | O_WRONLY, 0644);
    if(temp_fd == -1){
        perror("Error creating temp file");
        exit(EXIT_FAILURE);
    }
    char buf[BUFSIZ];
    char *msg_buf = NULL;
    char header_buf[sizeof(MessageHeader)];
    printf("hi, %lu\n", sizeof(MessageHeader));
    int ret;
    while((ret = read(_pub->db_fd, header_buf, sizeof(MessageHeader))) > 0){
        printf("hi from while\n");
        memcpy(&(_pub->msg_header), header_buf, sizeof(MessageHeader));
        printf("type:%i\npriority:%i\npubID:%i\nlenght:%i\n",
                _pub->msg_header.type,
                _pub->msg_header.has_prioriry,
                _pub->msg_header.publisherID,
                _pub->msg_header.len);
        msg_buf = (char*)realloc(msg_buf, (_pub->msg_header.len + 1) * sizeof(char));
        if(msg_buf == NULL){
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        read(_pub->db_fd, msg_buf, _pub->msg_header.len);
        printf("%s\n", msg_buf);
        //send to server


    }
    //exited while & messages unread remained
    while((ret = read(_pub->db_fd, buf, BUFSIZ)) > 0){
        write(temp_fd, buf, ret);
    }
    close(_pub->db_fd);
    _pub->db_fd = temp_fd;
    unlink(MSGFILENAME);
    rename(TEMPFILENAME, MSGFILENAME);


}
void closePubClient(Publisher* _pub){
    close(_pub->db_fd);
    close(_pub->socket_desc);
    free(_pub);
}


int main(void)
{
    Publisher* publisher_client = initServer(IPADDR, PORT);
 
    fetchMessages(publisher_client);
    closePubClient(publisher_client);
    
    return 0;
}