#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>

#define PUBLISHER_PORT 8000
#define SERVER_IPADDR "127.0.0.2"
#define SUBSCRIBER_PORT 8010
#define INITIALPUBQUEUECAP 4

typedef enum MsgType{
    MSG_BIN_DATA = 0,
    MSG_NOTIF,
    MSG_SYS_INFO,
    MSG_TASK,
    MSG_TERMINAL
}MsgType;


// type(int) - priority(bool) - pubID(int) - len(int) - data(len * char)
typedef struct{
    int len;
    int publisherID;
    bool has_prioriry;
    MsgType type;
}MessageHeader;

typedef struct{
    MessageHeader header;
    char* data;
}Message;



typedef struct{
    int subscribedID;
    MsgType type;
}Request;

#endif