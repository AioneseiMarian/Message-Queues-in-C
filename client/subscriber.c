#define _GNU_SOURCE

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../header/message.h"

#define SERVER_PORT 8080
#define SERVER_IPADDR "127.0.0.2"
#define MSGFILENAME "database/sub_db.json"


typedef struct {
    struct sockaddr_in server_addr;
    int server_fd;
    int db_fd
} Client;

Client* client;



int init_client() {
    client = malloc(sizeof(Client));

    client->server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client->server_fd < 0) {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(SERVER_PORT);
    client->server_addr.sin_addr.s_addr = inet_addr(SERVER_IPADDR);

    if (connect(client->server_fd, (struct sockaddr*)&client->server_addr,
                sizeof(client->server_addr)) < 0) {
        printf("Unable to connect\n");
        return -1;
    }

    return 0;
}


void fetch_from_db(Client *subscriber, const char* file_name) 
{
    subscriber->db_fd = open(file_name, O_RDONLY);
    if (subscriber->db_fd == -1) {
        perror("Error opening messages_queue file");
        exit(EXIT_FAILURE);
    }

    size_t file_length = lseek(subscriber->db_fd, 0, SEEK_END);
    lseek(subscriber->db_fd, 0, SEEK_SET);
    char *buffer = (char *)malloc((file_length + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    if (read(subscriber->db_fd, buffer, file_length) < 0) {
        perror("Error reading from messages_queue database");
        exit(EXIT_FAILURE);
    }
    close(subscriber->db_fd);

    struct json_object *parsed_array = json_tokener_parse(buffer);
    free(buffer);
    if (!parsed_array ||
        json_object_get_type(parsed_array) != json_type_array) {
        perror("Failed to parse JSON string");
        exit(EXIT_FAILURE);
    }

    size_t array_length = json_object_array_length(parsed_array);
    for (int i = 0; i < array_length; ++i) {
        struct json_object *message =
            json_object_array_get_idx(parsed_array, i);
        Message *msg = create_Message_From_Json(message);
        if(!msg){
            perror("Empty message. Can't push to queue");
        }
    }
}


void subscribe_to_topic(char topic[TOPICSIZ], char subtopic[SUBTOPICSIZ])
{

    json_object *json_message = create_Json_From_Message(
        MSG_SUBSCRIPTION, topic,
        subtopic, 5, "NULL");

	const char* json_string = json_object_to_json_string(json_message);
    printf("%s\n", json_string);
	size_t json_length = strlen(json_string);
	printf("About to send\n");
	ssize_t sent_bytes = send(client->server_fd, json_string, json_length, 0);
	if(sent_bytes < 0){
		perror("Sending message to server Failed");
	}else{
		printf("Sent %zd bytes successfully.\n", sent_bytes);
	}
	json_object_put(json_message);
}

int main(void) {
    if (init_client() == -1) {
        perror("");
        exit(-1);
    }
    char topic[TOPICSIZ];
    char subtopic[SUBTOPICSIZ];
    strcpy(topic, "Programming");
    strcpy(subtopic, "programming language");

    subscribe_to_topic(topic, subtopic);

    close(client->server_fd);

    return 0;
}
