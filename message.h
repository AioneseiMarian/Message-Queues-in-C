#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>

typedef enum{
    MSG_BIN_DATA = 0,
    MSG_NOTIF,
    MSG_SYS_INFO,
    MSG_TASK,
}MsgType;



typedef struct message
{
    int len;
    char* data;
    int publisherID;
    bool has_prioriry;
    MsgType type;
}Message;

#endif