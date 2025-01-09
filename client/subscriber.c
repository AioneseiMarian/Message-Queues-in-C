#define _GNU_SOURCE

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include "../header/message.h"

#define INIT_DATABASE "publisher_database/init_database.json"

typedef struct {
    struct sockaddr_in server_addr;
    int server_fd;
    int db_fd;
    int output_fd;
} Client;

Client* client;

void subscribe_to_topic(json_object* parsed_json);

void sigint_handler(int sig) {
    printf("\nCTRL + C detected.\nShutting down client\n");
    close(client->server_fd);
    close(client->output_fd);
    free(client);
    exit(0);
}

void setup_signal_handler() {
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
}

int init_client() {
    client = malloc(sizeof(Client));
    client->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->server_fd < 0) {
        printf("Unable to create socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    int database_fd = open(INIT_DATABASE, O_RDONLY);
    if (database_fd == -1) {
        perror("Error opening messages_queue file");
        exit(EXIT_FAILURE);
    }

    size_t file_length = lseek(database_fd, 0, SEEK_END);
    lseek(database_fd, 0, SEEK_SET);

    char *buffer = (char *)malloc((file_length + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    if (read(database_fd, buffer, file_length) < 0) {
        perror("Error reading from messages_queue database");
        exit(EXIT_FAILURE);
    }
    close(database_fd);
    printf("\nPrinting file content\n");
    printf("%s\n\n\n", buffer);

    json_object *parsed_array = json_tokener_parse(buffer);
    if (!parsed_array ||
        json_object_get_type(parsed_array) != json_type_array) {
        perror("Failed to parse JSON string");
        exit(EXIT_FAILURE);
    }
    free(buffer);
    
    json_object *json_ip, *json_port, *json_output_file;
    json_object_object_get_ex(json_object_array_get_idx(parsed_array, 0), "server_ip", &json_ip);
    json_object_object_get_ex(json_object_array_get_idx(parsed_array, 0), "server_port", &json_port);
    json_object_object_get_ex(json_object_array_get_idx(parsed_array, 0), "output_file", &json_output_file);

    const char *server_ip_addr = json_object_get_string(json_ip);
    int server_port = json_object_get_int(json_port);
    const char *output_file = json_object_get_string(json_output_file);
    printf("Server IP: %s\nServer Port: %d\nOutput file: %s\n", server_ip_addr, server_port, output_file);

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(server_port);
    client->server_addr.sin_addr.s_addr = inet_addr(server_ip_addr);


    if((client->output_fd = open(output_file, O_CREAT | O_WRONLY, 0644)) == -1){
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    if (connect(client->server_fd, (struct sockaddr*)&client->server_addr,
                sizeof(client->server_addr)) < 0) {
        printf("Unable to connect\n");
        return -1;
    }

    return 0;
}

struct json_object *fetch_from_db(const char* file_name) 
{
    client->db_fd = open(file_name, O_RDONLY);
    if (client->db_fd == -1) {
        perror("Error opening messages_queue file");
        exit(EXIT_FAILURE);
    }

    size_t file_length = lseek(client->db_fd, 0, SEEK_END);
    lseek(client->db_fd, 0, SEEK_SET);
    
    char *buffer = (char *)malloc((file_length + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    if (read(client->db_fd, buffer, file_length) < 0) {
        perror("Error reading from messages_queue database");
        exit(EXIT_FAILURE);
    }
    close(client->db_fd);
    printf("\nPrinting file content\n");
    printf("%s\n\n\n", buffer);

    printf("Parsing JSON string\n");
    struct json_object *parsed_array = json_tokener_parse(buffer);

    free(buffer);
    if (!parsed_array ||
        json_object_get_type(parsed_array) != json_type_array) {
        perror("Failed to parse JSON string");
        exit(EXIT_FAILURE);
    }


    size_t array_length = json_object_array_length(parsed_array);
    for (int i = 0; i < array_length; ++i) {
        struct json_object *message = json_object_array_get_idx(parsed_array, i);
        subscribe_to_topic(message);
    }
    printf("Subscribtions sent\n");

    return parsed_array;  
}

void subscribe_to_topic(json_object* parsed_json){
	const char* json_string = json_object_to_json_string(parsed_json);
    printf("%s\n", json_string);
	size_t json_length = strlen(json_string);
	printf("About to send\n");
	ssize_t sent_bytes = send(client->server_fd, json_string, json_length, 0);
	if(sent_bytes < 0){
		perror("Sending message to server Failed");
	}else{
		printf("Sent %zd bytes successfully.\n", sent_bytes);
	}
	json_object_put(parsed_json);
}

char* format_message(const char *message) {
    if (message == NULL) return NULL;  // Basic check for NULL input

    // Allocate buffer, assuming BUFSIZ is sufficient
    char *buffer = malloc(BUFSIZ);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    // Copy message into buffer
    strcpy(buffer, message);

    // Replace }{ with },{
    size_t i = 0;
    while (i < strlen(buffer) - 1) {  // -1 to ensure we don't read past the end
        if (buffer[i] == '}' && buffer[i + 1] == '{') {
            // Make room for the comma by shifting the rest of the string
            size_t length = strlen(buffer);
            memmove(buffer + i + 2, buffer + i + 1, length - i - 1 + 1);  // +1 for null terminator
            buffer[i + 1] = ',';
        }
        i++;
    }

    // Add brackets if they're not present (assuming they're needed for JSON array format)
    char *formatted = malloc(strlen(buffer) + 3);  // +3 for '[', ']', and '\0'
    if (formatted == NULL) {
        perror("Memory allocation for formatted string failed");
        free(buffer);
        return NULL;
    }
    sprintf(formatted, "[%s]", buffer);
    free(buffer);  // Free the original buffer since we've created a new one with brackets

    printf("Messages to be formatted: %s\n", message);
    printf("Formatted message: %s\n", formatted);
    return formatted;  // Caller must free this memory
}

void store_messages(const char* message){
    char* new_msg = format_message(message);

    json_object *parsed_json = json_tokener_parse(new_msg);
    if (!parsed_json) {
        perror("Failed to parse JSON string");
        exit(EXIT_FAILURE);
    }


    size_t array_length = json_object_array_length(parsed_json);
    json_object * json_array = json_object_new_array();
    for(int i = 0; i < array_length; ++i)
    {
        json_object *json_message = json_object_array_get_idx(parsed_json, i);
        json_object_array_add(json_array, json_message);
    }

    const char* json_string = json_object_to_json_string(json_array);
    size_t json_length = strlen(json_string);
    printf("Storing these messages to output file\n%s", json_string);
    if(write(client->output_fd, json_string, json_length) < 0){
        perror("Error writing to output file");
        exit(EXIT_FAILURE);
    }
    printf("Stored successfully\n");
    json_object_put(parsed_json);
}

void wait_for_messages(){
    char buf[BUFSIZ / 2];
    while(1) {
        sleep(1);
        ssize_t bytesRead = recv(client->server_fd, buf, sizeof(buf), 0);
        if (bytesRead < 0) {
            perror("Error reading from server");
            exit(EXIT_FAILURE);
        }
        if(bytesRead == 0){
            printf("Server closed connection\n");
            break;
        }
        buf[bytesRead] = '\0';
        printf("Received: %zd bytes\n", bytesRead);
        printf("The data: %s\n\n", buf);
        store_messages(buf);
    }
}

int main(int argc, char const *argv[]) {
    if(argc != 2){
        printf("Usage: %s file with subs file\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (init_client() == -1) {
        perror("");
        exit(-1);
    }
    setup_signal_handler();
    fetch_from_db(argv[1]);
    wait_for_messages();

    close(client->server_fd);

    return 0;
}
