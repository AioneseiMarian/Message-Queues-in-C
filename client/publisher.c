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
#include "../header/rbtree.h"

#define MSGSIZ 2048
#define MSGFILENAME "database/publ_db.json"

int pop_counter = 0;
int push_counter = 0;

typedef struct Publisher {
    int socket_fd;
    struct sockaddr_in server_addr;
    int db_fd;
    Queue_Node *messages_queue;
	RBTree* test_tree;
} Publisher;

void print_Message(Message *message) {
    printf("Type: %i\nTopic: %s\nSubtopic: %s\nLength: %i\nData: %s\n\n",
           message->header.msg_type, message->header.channel,
           message->header.topic, message->header.len, message->data);
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
	publisher->test_tree = create_Rbtree();

    publisher->server_addr.sin_family = AF_INET;
    publisher->server_addr.sin_port = htons(_port);
    publisher->server_addr.sin_addr.s_addr = inet_addr(_addr);

    publisher->db_fd = open(MSGFILENAME, O_RDONLY);
    if (publisher->db_fd == -1) {
        perror("Error opening messages_queue file");
        exit(EXIT_FAILURE);
    }
    /* if (connect(publisher->socket_fd, */
    /*             (struct sockaddr *)&(publisher->server_addr), */
    /*             sizeof(publisher->server_addr)) < 0) { */
    /*     perror("Unable to connect"); */
    /*     exit(EXIT_FAILURE); */
    /* } */
    /* printf("Connected with server successfully\n"); */

    return publisher;
}
void fetch_All_From_DB(Publisher *publisher) {
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
			push_counter++;
			printf("Push number %i:\n", push_counter);
			push_Queue(&(publisher->messages_queue), msg);
			insert_Rbt(publisher->test_tree, msg);
		}else{
			perror("Empty message. Can't push to queue");
		}
	}
}
void send_message(Publisher *publisher) {
	if (!publisher->messages_queue) {
		fprintf(stderr, "Queue is empty. No message to send.\n");
		return;
	}

    Message *msg = pop_Queue(&(publisher->messages_queue));
	RBTNode* found_Rbt_node = search_Rbt(publisher->test_tree, publisher->test_tree->root, "File operations");
	Message* msg_rbt = found_Rbt_node->msg;
	delete_Rbt(publisher->test_tree, found_Rbt_node);
	printf("Message popped from queue:\n");
    print_Message(msg);
	printf("Message extracted from rbtree:\n");
	print_Message(msg_rbt);
    if (msg == NULL) {
        fprintf(stderr, "Empty queue. Can't send to server");
        return;
    }

    json_object *json_message = create_Json_From_Message(
        msg->header.msg_type, msg->header.channel,
        msg->header.topic, msg->header.len, msg->data);
    free(msg);

	// Sending to socket
	json_object_put(json_message);
}
void send_All_Messages(Publisher* publisher){
	while (publisher->messages_queue != NULL){
		pop_counter++;
		printf("Sending message number %i:\n", pop_counter);
		send_message(publisher);
	}
}
void close_Publisher_Client(Publisher *publisher) {
    close(publisher->db_fd);
    close(publisher->socket_fd);
	free_Queue(&(publisher->messages_queue));
    free(publisher);
}
int main(void) {
    setbuf(stdin, NULL);
    Publisher *publisher_client = init_Server(SERVER_IPADDR, SERVER_PORT);
    fetch_All_From_DB(publisher_client);
	send_All_Messages(publisher_client);
    /* close_Publisher_Client(publisher_client); */

    return 0;
}
