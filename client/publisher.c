#define _GNU_SOURCE	

#include <arpa/inet.h>
#include <fcntl.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../header/message.h"
#include "../header/queues.h"

#define MSGSIZ 2048
#define MSGFILENAME "database/publ_db.json"

typedef struct Publisher {
    int socket_fd;
    struct sockaddr_in server_addr;
    int db_fd;
    Queue_Node *messages_queue;
} Publisher;

void print_Message(Message *message) {
    printf("Type: %i\nTopic: %s\nSubtopic: %s\nLength: %i\nData: %s\n\n",
           message->header.msg_type, message->header.topic,
           message->header.subtopic, message->header.len, (char*)message->data);
}
Publisher *init_Server(char *_addr, int _port) {
    Publisher *publisher = (Publisher *)malloc(sizeof(Publisher));
    if (publisher == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    publisher->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (publisher->socket_fd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    publisher->messages_queue = NULL;

    publisher->server_addr.sin_family = AF_INET;
    publisher->server_addr.sin_port = htons(_port);
    publisher->server_addr.sin_addr.s_addr = inet_addr(_addr);

    if (connect(publisher->socket_fd,
                (struct sockaddr *)&(publisher->server_addr),
                sizeof(publisher->server_addr)) < 0) {
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }
    printf("Connected with server successfully\n");

    return publisher;
}
void fetch_All_From_DB(Publisher *publisher, const char* file_name) {
    publisher->db_fd = open(file_name, O_RDONLY);
    if (publisher->db_fd == -1) {
        perror("Error opening messages_queue file");
        exit(EXIT_FAILURE);
    }

    size_t file_length = lseek(publisher->db_fd, 0, SEEK_END);
    lseek(publisher->db_fd, 0, SEEK_SET);
    char *buffer = (char *)malloc((file_length + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    if (read(publisher->db_fd, buffer, file_length) < 0) {
        perror("Error reading from messages_queue database");
        exit(EXIT_FAILURE);
    }
    close(publisher->db_fd);

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
		if(msg){
			push_Queue(&(publisher->messages_queue), msg);
		}else{
			perror("Empty message. Can't push to queue");
		}
	}
}
void send_Message(Publisher *publisher) {
	if (!publisher->messages_queue) {
		fprintf(stderr, "Queue is empty. No message to send.\n");
		return;
	}
    Message *msg = pop_Queue(&(publisher->messages_queue));
    if (msg == NULL) {
        fprintf(stderr, "Empty queue. Can't send to server");
        return;
    }
	printf("\n\nSending message:\n");
	print_Message(msg);

    json_object *json_message = create_Json_From_Message(
        msg->header.msg_type, msg->header.topic,
        msg->header.subtopic, msg->header.len, msg->data);
    free(msg);

	const char* json_string = json_object_to_json_string(json_message);
	size_t json_length = strlen(json_string);
	printf("About to send\n");
	ssize_t sent_bytes = send(publisher->socket_fd, json_string, json_length, 0);
	if(sent_bytes < 0){
		perror("Sending message to server Failed");
	}else{
		printf("Sent %zd bytes successfully.\n", sent_bytes);
	}

	json_object_put(json_message);
}
void send_All_Messages(Publisher* publisher){
	while (publisher->messages_queue != NULL){
		send_Message(publisher);
	}
}
void close_Publisher_Client(Publisher *publisher) {
    close(publisher->db_fd);
    close(publisher->socket_fd);
	free_Queue(&(publisher->messages_queue));
    free(publisher);
}
int main(int argc, char** argv) {
    Publisher *publisher_client = init_Server(SERVER_IPADDR, SERVER_PORT);
	if(argc != 2){
		fprintf(stderr, "Invalid param\n");
		exit(EXIT_FAILURE);
	}
	fetch_All_From_DB(publisher_client, argv[1]);
	send_All_Messages(publisher_client);
    close_Publisher_Client(publisher_client);

    return 0;
}
