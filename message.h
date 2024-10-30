#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>

typedef enum{
    MSG_BIN_DATA = 0,
    MSG_NOTIF,
    MSG_SYS_INFO,
    MSG_TASK,
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

#endif