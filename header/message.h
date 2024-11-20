#ifndef MESSAGE_H
#define MESSAGE_H

#define SERVER_IPADDR "127.0.0.2"
#define SERVER_PORT 8080
#define CHANNELSIZ 40
#define TOPICSIZ 40

typedef enum MsgType{
	MSG_SUBSCRIPTION,
	MSG_PUBLISHING,
	MSG_NOTIFICATION,
}MsgType;

typedef struct{
    int len;
    MsgType msg_type;
	char channel[CHANNELSIZ];
	char topic[TOPICSIZ];
	/* int priority; */
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
