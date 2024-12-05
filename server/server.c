#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../header/hash_table.h"
#define MAX_EVENTS 64
#define INITIALPUBQUEUECAP 4

typedef struct Client {
    struct sockaddr_in client_addr;
    int client_fd;
    unsigned int client_size;
} Client;

typedef struct Server {
    int server_fd;
    struct sockaddr_in server_addr;
    HashTable* messages;
} Server;

void set_Non_Blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("Error getting socket flags");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error setting socket to non-blocking");
        exit(EXIT_FAILURE);
    }
}
Server* init_Server(char* _addr, int _port) {
    Server* server = (Server*)malloc(sizeof(Server));
    if (server == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
	int opt = 1;
	if(setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		perror("Setsockopt failed");
		close(server->server_fd);
		exit(EXIT_FAILURE);
	}
    printf("Socket created succesfully\n");

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_port = htons(_port);
    server->server_addr.sin_addr.s_addr = inet_addr(_addr);

    server->messages = create_Hashtable();

    if (bind(server->server_fd, (struct sockaddr*)&(server->server_addr),
             sizeof(server->server_addr)) < 0) {
        perror("Error on binding");
        exit(EXIT_FAILURE);
    }
    if (listen(server->server_fd, 10) < 0) {
        perror("Error on listening");
        close(server->server_fd);
        exit(EXIT_FAILURE);
    }
    return server;
}
void listenClient(Server* server, Client* client) {
    if (listen(server->server_fd, 1) < 0) {
        perror("Error while listening");
        exit(EXIT_FAILURE);
    }
    printf("Listening client . . . \n");
    client->client_size = sizeof(client->client_addr);
    client->client_fd = accept(server->server_fd,
                               (struct sockaddr*)&(client->client_addr),
                               &(client->client_size));
    if (client->client_fd < 0) {
        perror("Can't accept publisher");
        exit(EXIT_FAILURE);
    }
    printf("Publisher connected at IP: %s and port: %i\n",
           inet_ntoa(client->client_addr.sin_addr),
           ntohs(client->client_addr.sin_port));
}
void close_Server(Server* server) {
    close(server->server_fd);
	free_Hashtable(server->messages);
    free(server);
}
void store_Message(Server* server, Message* msg){
    insert_Hashtable(server->messages, msg->header.topic, msg->header.subtopic, msg);
}
Message* retrieve_Message(Server* server, const char* topic, const char* subtopic){
	unsigned int index = hash_Function(topic);
	HashTableEntry* entry = server->messages->buckets[index];
	HashTableEntry* prev = NULL;
	while(entry){
		if(strcmp(entry->topic, topic) == 0){
			Queue_Node* messages = search_Rbt(entry->tree, subtopic);
			if(messages){
				void* data = pop_Queue(&messages);
				if(messages == NULL){
					delete_Rbt(entry->tree, subtopic);
				}
				if(entry->tree->root == entry->tree->NIL){
					if(prev){
						prev->next = entry->next;
					}else{
						server->messages->buckets[index] = entry->next;
					}
					free_Rbt(entry->tree);
					free(entry);
				}
				return (Message*)data;
			}
			fprintf(stderr, "Subtopic '%s' is not found under topic '%s'\n", subtopic, topic);
			return NULL;
		}
		prev = entry;
		entry = entry->next;
	}
	fprintf(stderr, "Topic '%s' not found in the hash table\n", topic);
	return NULL;
}
void handle_Publishing(Server* server, struct json_object* parsed_msg) {
    Message* msg = create_Message_From_Json(parsed_msg);
	store_Message(server, msg);
	/* print_Hashtable(server->messages); */
}
void handle_Subscription(Server* server, struct json_object* parsed_msg) {
}
void handle_Notification(Server* server, struct json_object* parsed_msg) {
}
void process_json_message(Server* server, json_object* parsed_msg) {
    MsgType msg_type;
    struct json_object* type;
    if (json_object_object_get_ex(parsed_msg, "type", &type)) {
        msg_type = json_object_get_int(type);
    }
    switch (msg_type) {
        case MSG_PUBLISHING:
            handle_Publishing(server, parsed_msg);
            break;
        case MSG_SUBSCRIPTION:
            handle_Subscription(server, parsed_msg);
            break;
        case MSG_NOTIFICATION:
            handle_Notification(server, parsed_msg);
            break;
        default:
            fprintf(stderr, "Invalid message type");
    }
}
void parse_received_json(Server* server, char* json_string) {
    struct json_tokener* tok = json_tokener_new();
    struct json_object* json_msg;
    enum json_tokener_error jerr;

    char* start = json_string;
    while (*start) {
        json_msg = json_tokener_parse_ex(tok, start, strlen(start));
        jerr = json_tokener_get_error(tok);
        if (jerr == json_tokener_success) {
            process_json_message(server, json_tokener_parse(json_string));
            start += (tok->char_offset);
            json_object_put(json_msg);
        } else if (jerr == json_tokener_continue) {
            break;
        } else {
            fprintf(stderr, "JSON parsing error: %s\n", json_tokener_error_desc(jerr));
            ++start;
        }
    }
    size_t remaining = strlen(start);
    memmove(json_string, start, remaining);
    json_string[remaining] = '\0';

    json_tokener_free(tok);
}
void handle_Client_Read(Server* server, int client_fd) {
    char buf[BUFSIZ / 2];
    while (1) {
        int bytesRead = read(client_fd, buf, sizeof(buf));
        if (bytesRead == 0) {
            printf("Client disconnected\n");
            close(client_fd);
            break;
        } else if (bytesRead < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("Error reading from client");
                close(client_fd);
                break;
            }
        } else {
            buf[bytesRead] = '\0';
            printf("Received: %i\n", bytesRead);
            parse_received_json(server, buf);
        }
    }
}

void start_Epoll_Server(Server* server) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Error creating epoll instance");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server->server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server->server_fd, &ev) == -1) {
        perror("Error adding server socket to epoll");
        exit(EXIT_FAILURE);
    }

    set_Non_Blocking(server->server_fd);
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Error during epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server->server_fd) {
                while (1) {
                    Client* client = (Client*)malloc(sizeof(Client));
                    if (!client) {
                        perror("Error allocating client memory");
                        continue;
                    }

                    client->client_size = sizeof(client->client_addr);
                    client->client_fd = accept(server->server_fd,
                                               (struct sockaddr*)&client->client_addr,
                                               &client->client_size);
                    if (client->client_fd < 0) {
                        free(client);
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Toate conexiunile au fost acceptate.
                            break;
                        } else {
                            perror("Can't accept connection");
                        }
                        continue;
                    }

                    printf("New client connected\n");
                    set_Non_Blocking(client->client_fd);

                    ev.events = EPOLLIN | EPOLLET;  // Edge Triggered pentru client.
                    ev.data.fd = client->client_fd;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client->client_fd, &ev) == -1) {
                        perror("Error adding client socket to epoll");
                        close(client->client_fd);
                        free(client);
                        continue;
                    }
                }
            } else {
                handle_Client_Read(server, events[i].data.fd);
            }
        }
    }
}

int main() {
    setbuf(stdout, NULL);
    Server* server = init_Server(SERVER_IPADDR, SERVER_PORT);
    start_Epoll_Server(server);

    // testMessageQueues(server);
    // testMessageQueues(server);

    // close_Server(server);
    return 0;
}
