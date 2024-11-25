#include <stdio.h>
#include <string.h>

#include "../header/message.h"

json_object *create_Json_From_Message(MsgType type, char *topic, char *topic_subtype, int len, char *data) {
    json_object *message = json_object_new_object();
    json_object_object_add(message, "type", json_object_new_int(type));
    json_object_object_add(message, "topic", json_object_new_string(topic));
    json_object_object_add(message, "message_type", json_object_new_string(topic_subtype));
    json_object_object_add(message, "length", json_object_new_int(len));
    json_object_object_add(message, "data", json_object_new_string(data));
    return message;
}
Message *create_Message_From_Json(json_object *json_msg) {
	Message* msg = malloc(sizeof(Message));
	if (!msg) {
		perror("Failed to allocate message");
		return NULL;
	}
	json_object* type_obj, *topic_obj, *msg_type_obj, *length_obj, *data_obj;
		
	json_object_object_get_ex(json_msg, "type", &type_obj);
	json_object_object_get_ex(json_msg, "topic", &topic_obj);
	json_object_object_get_ex(json_msg, "message_type", &msg_type_obj);
	json_object_object_get_ex(json_msg, "length", &length_obj);
	json_object_object_get_ex(json_msg, "data", &data_obj);

	msg->header.msg_type = json_object_get_int(type_obj);
	msg->header.len = json_object_get_int(length_obj);
	strncpy(msg->header.channel, json_object_get_string(topic_obj), CHANNELSIZ - 1);
	strncpy(msg->header.topic, json_object_get_string(msg_type_obj), TOPICSIZ - 1);
										
	const char* data_str = json_object_get_string(data_obj);
	msg->data = strdup(data_str);

	return msg;
}
