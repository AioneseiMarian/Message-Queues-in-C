#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../header/message.h"

#define MSG1 "Hello world"
#define MSG2 "The quick brown fox jumps over the lazy dog.?!#$^0123456789"
#define MSG3 "ding dong you have to change your google password"
#define MSG4 "CPU temp:41 Celsius | RAM: 26%"
#define MSG5 "GET INFO ALL DB2"

void writter(int fd, MsgType type, bool priority, int pulbID, int len,
             char* data) {
    char* buf = (char*)malloc((sizeof(MessageHeader) + len) * sizeof(char));
    Message msg = {
        .header.type = type,
        .header.has_prioriry = priority,
        .header.publisherID = pulbID,
        .header.len = len,
        .data = (char*)malloc(len * sizeof(char)),
    };
    memcpy(msg.data, data, len);

    memcpy(buf, &msg.header, sizeof(MessageHeader));
    memcpy(buf + sizeof(MessageHeader), msg.data, msg.header.len);

    write(fd, buf, sizeof(MessageHeader) + len);
}

int main() {
    int fd = open("database/publ_db.txt", O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("Err opening file");
        exit(EXIT_FAILURE);
    }
    writter(fd, MSG_NOTIF, 0, 1, strlen(MSG1), MSG1);
    writter(fd, MSG_BIN_DATA, 0, 3, strlen(MSG2), MSG2);
    writter(fd, MSG_NOTIF, 1, 2, strlen(MSG3), MSG3);
    writter(fd, MSG_SYS_INFO, 0, 12, strlen(MSG4), MSG4);
    writter(fd, MSG_TASK, 0, 5, strlen(MSG5), MSG5);

    close(fd);
    return 0;
}
