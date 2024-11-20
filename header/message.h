#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>

#define PUBLISHER_PORT 8000
#define SERVER_IPADDR "127.0.0.2"
#define SUBSCRIBER_PORT 8010

typedef enum MsgType{
}MsgType;


typedef struct{
    int len;
    MsgType type;
	int priority;
}MessageHeader;

typedef struct{
    MessageHeader header;
    char* data;
}Message;


typedef struct{
    int subscribedID;
    char type[20];
}Request;

#endif
