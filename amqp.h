#ifndef AMQP_H
#define AMQP_H

#include <stdbool.h>

typedef enum{
    MSG_BIN_DATA,
    MSG_NOTIF,
    MSG_SYS_INFO,
    MSG_TASK
}MessageType;



typedef struct{
    int publisherID;
    int len;
    char* data;
    MessageType msgType;
    bool important;
}Message;



#endif