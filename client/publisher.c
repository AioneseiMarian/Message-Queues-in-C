#define _GNU_SOURCE

#include <arpa/inet.h>
#include <fcntl.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../header/message.h"

#define SERVER_IPADDR "127.0.0.2"
#define SERVER_PORT 8000
#define MSGSIZ 2048
#define MSGFILENAME "database/publ_db.txt"
#define TEMPFILENAME "database/publ_db2.txt"

char *getStringType(MsgType _type) {
    switch (_type) {
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

typedef struct Publisher {
    int socket_fd;
    struct sockaddr_in server_addr;
    MessageHeader msg_header;
    int db_fd;
} Publisher;
Publisher *initServer(char *_addr, int _port) {
    Publisher *_pub = (Publisher *)malloc(sizeof(Publisher));
    if (_pub == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    _pub->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_pub->socket_fd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    _pub->server_addr.sin_family = AF_INET;
    _pub->server_addr.sin_port = htons(_port);
    _pub->server_addr.sin_addr.s_addr = inet_addr(_addr);

    _pub->db_fd = open(MSGFILENAME, O_RDONLY);
    if (_pub->db_fd == -1) {
        perror("Error opening messages file");
        exit(EXIT_FAILURE);
    }
    if (connect(_pub->socket_fd, (struct sockaddr *)&(_pub->server_addr),
                sizeof(_pub->server_addr)) < 0) {
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }
    printf("Connected with server successfully\n");

    return _pub;
}

void printHeader(MessageHeader *_hdr) {
    printf("type: %s\npriority: %s\npubID: %i\nlenght: %i\n",
           getStringType(_hdr->type), _hdr->has_prioriry ? "High" : "Low",
           _hdr->publisherID, _hdr->len);
}
void sendTerminalMessage(Publisher *_pub) {
    MessageHeader tmp = {
        .has_prioriry = 0,
        .len = 0,
        .publisherID = 0,
        .type = MSG_TERMINAL,
    };
    char buf[sizeof(MessageHeader)];
    memcpy(buf, &tmp, sizeof(MessageHeader));
    if (send(_pub->socket_fd, buf, sizeof(MessageHeader), 0) < 0) {
        printf("Unable to send message to server\n");
    }
}
void fetchAndPublish(Publisher *_pub) {
    if (lseek(_pub->db_fd, 0, SEEK_END) == 0) {
        perror("Empty database");
        exit(EXIT_FAILURE);
    }
    lseek(_pub->db_fd, 0, SEEK_SET);
    char header_buf[sizeof(MessageHeader)];
    bool was_modified = 0;
    int ret;
    while ((ret = read(_pub->db_fd, header_buf, sizeof(MessageHeader))) > 0) {
        was_modified = 1;
        memcpy(&(_pub->msg_header), header_buf, sizeof(MessageHeader));
        // debug //
        printHeader(&_pub->msg_header);
        char *data_buf = (char *)malloc(
            (_pub->msg_header.len + sizeof(MessageHeader)) * sizeof(char));
        if (data_buf == NULL) {
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        memset(data_buf, '\0', (_pub->msg_header.len + sizeof(MessageHeader)));
        read(_pub->db_fd, data_buf + sizeof(MessageHeader),
             _pub->msg_header.len);
        memcpy(data_buf, header_buf, sizeof(MessageHeader));
        // debug //
        printf("%s\n---------------------------\n",
               data_buf + sizeof(MessageHeader));

        if (send(_pub->socket_fd, data_buf,
                 _pub->msg_header.len + sizeof(MessageHeader), 0) < 0) {
            printf("Unable to send message to server\n");
        }
        free(data_buf);
        data_buf = NULL;
    }
    sendTerminalMessage(_pub);
    if (was_modified) {
        int temp_fd = open(TEMPFILENAME, O_CREAT | O_WRONLY, 0644);
        if (temp_fd == -1) {
            perror("Error creating temp file");
            exit(EXIT_FAILURE);
        }
        char buf[BUFSIZ];
        while ((ret = read(_pub->db_fd, buf, BUFSIZ)) > 0) {
            write(temp_fd, buf, ret);
        }
        close(_pub->db_fd);
        _pub->db_fd = temp_fd;
        unlink(MSGFILENAME);
        rename(TEMPFILENAME, MSGFILENAME);
    }
}
void closePubClient(Publisher *_pub) {
    close(_pub->db_fd);
    close(_pub->socket_fd);
    free(_pub);
}
int main(void) {
    setbuf(stdout, NULL);
    Publisher *publisher_client = initServer(SERVER_IPADDR, SERVER_PORT);

    fetchAndPublish(publisher_client);
    closePubClient(publisher_client);

    return 0;
}
