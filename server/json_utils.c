#include <stdio.h>
#include <string.h>

#include "../header/message.h"

json_object *create_Json_From_Message(MsgType type, char *topic, char *subtopic, int len, char *data) {
    json_object *message = json_object_new_object();
    json_object_object_add(message, "type", json_object_new_int(type));
    json_object_object_add(message, "topic", json_object_new_string(topic));
    json_object_object_add(message, "subtopic", json_object_new_string(subtopic));
    json_object_object_add(message, "length", json_object_new_int(len));
    json_object_object_add(message, "data", json_object_new_string(data));
    return message;
}

json_object *create_Json_From_Subscribtion(char *topic, char *subtopic, int len, char* interest) {
    json_object *message = json_object_new_object();
    json_object_object_add(message, "topic", json_object_new_string(topic));
    json_object_object_add(message, "subtopic", json_object_new_string(subtopic));
    json_object_object_add(message, "length", json_object_new_int(len));
    json_object_object_add(message, "interest", json_object_new_string(interest));
    return message;
}

Message *create_Message_From_Json(json_object *json_msg) {
	Message* msg = malloc(sizeof(Message));
	if (!msg) {
		perror("Failed to allocate message");
		return NULL;
	}
	json_object* type_obj, *topic_obj, *subtopic_obj, *length_obj, *data_obj;
		
	json_object_object_get_ex(json_msg, "type", &type_obj);
	json_object_object_get_ex(json_msg, "topic", &topic_obj);
	json_object_object_get_ex(json_msg, "subtopic", &subtopic_obj);
	json_object_object_get_ex(json_msg, "length", &length_obj);
	json_object_object_get_ex(json_msg, "data", &data_obj);

	msg->header.new = 1; 		
	msg->header.msg_type = json_object_get_int(type_obj);
	msg->header.len = json_object_get_int(length_obj);
	strncpy(msg->header.topic, json_object_get_string(topic_obj), TOPICSIZ- 1);
	strncpy(msg->header.subtopic, json_object_get_string(subtopic_obj), SUBTOPICSIZ- 1);
										
	const char* data_str = json_object_get_string(data_obj);
	msg->data = strdup(data_str);

	return msg;
}

Subscribtion *create_Subscribtion_From_Json(json_object *json_msg, int client_fd){
	Subscribtion *sub = malloc(sizeof(Subscribtion));
	if(!sub){
		perror("Failed to allocate message");
		return NULL;
	}
	json_object *topic_obj, *subtopic_obj, *interest_obj, *size_obj;
	
	json_object_object_get_ex(json_msg, "topic", &topic_obj);			
	json_object_object_get_ex(json_msg, "subtopic", &subtopic_obj);
	json_object_object_get_ex(json_msg, "interest", &interest_obj);
	json_object_object_get_ex(json_msg, "size", &size_obj);

	if(interest_obj == NULL){
		sub->has_interest = 0;
	}else{
		sub->has_interest = 1;
	}

	sub->client_fd = client_fd;
	sub->len = json_object_get_int(size_obj);
	sub->new = 1;
	strncpy(sub->topic, json_object_get_string(topic_obj), TOPICSIZ - 1);
	strncpy(sub->subtopic, json_object_get_string(subtopic_obj), SUBTOPICSIZ- 1);
	strncpy(sub->interest, json_object_get_string(interest_obj), INTERESTSIZ- 1);
	
	
	return sub;
}

