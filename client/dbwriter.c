#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h> 

#include "../header/message.h"

#define TOPIC1 "Programming"
#define MSG1 "Hello world"
#define MSG2 "The C programming language is a multi-purpose ..."

#define TOPIC2 "File operations"
#define MSG3 "create fis1.txt"
#define MSG4 "create fis2.txt"
#define MSG5 "remove fis1.txt"

#define TOPIC3 "Shop offers"
#define MSG6 "promo for pork"
#define MSG7 "soda 10% off"

void writter(int fd) {
	struct json_object *_js_array = json_object_new_array();
	json_object_array_add(_js_array, create_Json_From_Message(MSG_PUBLISHING, TOPIC1, "program output", strlen(MSG1), MSG1));
	json_object_array_add(_js_array, create_Json_From_Message(MSG_PUBLISHING, TOPIC1, "programming language", strlen(MSG2), MSG2));
	json_object_array_add(_js_array, create_Json_From_Message(MSG_PUBLISHING, TOPIC2, "file creation", strlen(MSG3), MSG3));
	json_object_array_add(_js_array, create_Json_From_Message(MSG_PUBLISHING, TOPIC2, "file creation", strlen(MSG4), MSG4));
	json_object_array_add(_js_array, create_Json_From_Message(MSG_PUBLISHING, TOPIC2, "file deletion", strlen(MSG5), MSG5));
	json_object_array_add(_js_array, create_Json_From_Message(MSG_PUBLISHING, TOPIC3, "meat promo", strlen(MSG6), MSG6));
	json_object_array_add(_js_array, create_Json_From_Message(MSG_PUBLISHING, TOPIC3, "soda promo", strlen(MSG7), MSG7));

	const char* json_string = json_object_to_json_string_ext(_js_array, JSON_C_TO_STRING_PRETTY);
	write(fd, json_string, strlen(json_string));
	
	json_object_put(_js_array);
}

int main() {
    int fd = open("database/publ_db.json", O_WRONLY | O_CREAT | O_TRUNC| O_APPEND, 0664);
    if (fd == -1) {
        perror("Err opening file");
        exit(EXIT_FAILURE);
    }
	writter(fd);
    close(fd);
    return 0;
}
