#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include"../header/message.h"

void writter(int fd, MsgType type, bool priority, int pulbID, int len, char* data){
    char* buf = (char*)malloc((sizeof(MessageHeader)+len)*sizeof(char));
    Message msg = {
        .header.type = type,
        .header.has_prioriry = priority,
        .header.publisherID = pulbID,
        .header.len = len,
        .data = (char*)malloc(len*sizeof(char)),
    };
    memcpy(msg.data, data, len);

    memcpy(buf, &msg.header, sizeof(MessageHeader));
    memcpy(buf + sizeof(MessageHeader), msg.data, msg.header.len);

    write(fd, buf, sizeof(MessageHeader)+len);
}

int main(){
    int fd = open("publ_db.txt", O_WRONLY | O_APPEND);
    if(fd == -1){
        perror("Err opening file");
        exit(EXIT_FAILURE);
    }
    writter(fd, MSG_NOTIF, 0, 1, strlen("Hello world"), "Hello world");
    writter(fd, TERMINATE, 0, 1, strlen("stop"), "stop");

    close(fd);
    return 0;
}