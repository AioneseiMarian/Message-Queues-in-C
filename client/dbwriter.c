#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h> 

#include "../header/message.h"

void writter(json_object* _js_array) {
	
}

int main() {
    int fd = open("database/publ_db.json", O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("Err opening file");
        exit(EXIT_FAILURE);
    }
	struct json_object *js_array = json_object_new_array();

    close(fd);
    return 0;
}
