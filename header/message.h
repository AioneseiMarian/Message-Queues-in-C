#ifndef MESSAGE_H
#define MESSAGE_H

#define SERVER_IPADDR "127.0.0.2"
#define SERVER_PORT 8080
#define TOPICSIZ 40
#define SUBTOPICSIZ 40
#define INTERESTSIZ 40


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
	char new;							//Added for controlling which messages we send to all subscribers and which only to the newly connected ones
										//if it's new we send to all, if it's not new we send it only to new subscribers
}MessageHeader;

typedef struct{
    MessageHeader header;
    void* data;
}Message;

typedef struct{
	int client_fd;
	int len;
	char topic[TOPICSIZ];
	char subtopic[SUBTOPICSIZ];
	char interest[INTERESTSIZ];
	char new;							//Same as for messages, if it's new sub, we send him all messages, if not, we only send him the new ones.
}Subscribtion;


json_object* create_Json_From_Message(MsgType type, char* topic, char* subtopic, int len, char* data);
Message* create_Message_From_Json(json_object *j_obj);

Subscribtion *create_Subscribtion_From_Json(json_object *json_msg, int client_fd);
json_object *create_Json_From_Subscribtion(char *topic, char *subtopic, int len, char* interest);



#endif
