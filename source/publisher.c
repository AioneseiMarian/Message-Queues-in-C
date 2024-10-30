#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../header/message.h"

#define SERVER_IPADDR "127.0.0.1"
#define SERVER_PORT 8000
#define MSGSIZ 2048
#define MSGFILENAME "publ_db.txt"
#define TEMPFILENAME "publ_db2.txt"

typedef struct Publisher{
    int socket_fd;
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
    _pub->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_pub->socket_fd < 0){
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
    if(connect(_pub->socket_fd, (struct sockaddr*)&(_pub->server_addr), sizeof(_pub->server_addr)) < 0){
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }
    printf("Connected with server successfully\n");

    return _pub;
}

void fetchAndSend(Publisher* _pub){
    if(lseek(_pub->db_fd, 0, SEEK_END) == 0){
        printf("Empty database");
        exit(EXIT_FAILURE);
    }
    lseek(_pub->db_fd, 0, SEEK_SET);
    char buf[BUFSIZ];
    char *msg_buf = NULL;
    char header_buf[sizeof(MessageHeader)];
    int ret;
    bool was_modified = 0;
    while((ret = read(_pub->db_fd, header_buf, sizeof(MessageHeader))) > 0){
        was_modified = 1;
        memcpy(&(_pub->msg_header), header_buf, sizeof(MessageHeader));
        printf("type:%i\npriority:%i\npubID:%i\nlenght:%i\n",
                _pub->msg_header.type,
                _pub->msg_header.has_prioriry,
                _pub->msg_header.publisherID,
                _pub->msg_header.len);
        msg_buf = (char*)realloc(msg_buf, (_pub->msg_header.len + sizeof(MessageHeader)) * sizeof(char));
        if(msg_buf == NULL){
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        read(_pub->db_fd, msg_buf + sizeof(MessageHeader), _pub->msg_header.len);
        memcpy(msg_buf, header_buf, sizeof(MessageHeader));
        printf("%s\n", msg_buf+sizeof(MessageHeader));
        //send to server
        if(send(_pub->socket_fd, msg_buf, _pub->msg_header.len + sizeof(MessageHeader), 0) < 0){
            printf("Unable to send message to server\n");
        }
        free(msg_buf);
    }
    if(was_modified){
        int temp_fd = open(TEMPFILENAME, O_CREAT | O_WRONLY, 0644);
        if(temp_fd == -1){
            perror("Error creating temp file");
            exit(EXIT_FAILURE);
        }
        while((ret = read(_pub->db_fd, buf, BUFSIZ)) > 0){
            write(temp_fd, buf, ret);
        }
        close(_pub->db_fd);
        _pub->db_fd = temp_fd;
        unlink(MSGFILENAME);
        rename(TEMPFILENAME, MSGFILENAME);
    }

}
void closePubClient(Publisher* _pub){
    close(_pub->db_fd);
    close(_pub->socket_fd);
    free(_pub);
}


int main(void)
{
    setbuf(stdout, NULL);
    Publisher* publisher_client = initServer(SERVER_IPADDR, SERVER_PORT);
 
    fetchAndSend(publisher_client);
    closePubClient(publisher_client);
    
    return 0;
}