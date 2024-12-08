#ifndef MESSAGE_H
#define MESSAGE_H

#define SERVER_IPADDR "127.0.0.2"
#define SERVER_PORT 8080
#define TOPICSIZ 40
#define SUBTOPICSIZ 40


#include <json-c/json.h>

typedef enum MsgType{
	MSG_SUBSCRIPTION,
	MSG_PUBLISHING,
	MSG_ALERT,
}MsgType;

typedef struct{
    int len;
    MsgType msg_type;
	char topic[TOPICSIZ];
	char subtopic[SUBTOPICSIZ];
}MessageHeader;

typedef struct{
    MessageHeader header;
    void* data;
}Message;


json_object* create_Json_From_Message(MsgType type, char* topic, char* subtopic, int len, char* data);
Message* create_Message_From_Json(json_object *j_obj);
Message *create_Subscribtion_From_Json(json_object *json_msg, int client_fd);

#endif
