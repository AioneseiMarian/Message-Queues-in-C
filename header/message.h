#ifndef MESSAGE_H
#define MESSAGE_H

#define SERVER_IPADDR "127.0.0.2"
#define SERVER_PORT 8080
#define CHANNELSIZ 40
#define TOPICSIZ 40

#include <json-c/json.h>

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

json_object* create_Json_From_Message(MsgType type, char* topic, char* topic_subtype, int len, char* data);
Message* create_Message_From_Json(json_object *j_obj);

#endif
