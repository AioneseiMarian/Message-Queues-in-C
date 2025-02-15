#include <netinet/in.h>
#define _GNU_SOURCE

#include "../header/server.h"
#include "../header/message_send.h"


atomic_bool should_terminate = ATOMIC_VAR_INIT(false);

pthread_t recv_thread[WORKER_THREADS];
pthread_t thread_send_msg;

Server* global_server;              //for deallocating memory at shutdown

pthread_mutex_t client_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

Queue_Node *clients_fd_queue = NULL;      //for closing all connections at shutdown

void store_Message(Server* server, Message* msg);

void load_messages_from_database(Server* server){
    printf("\nLoading messages from server_database/messages.json\n");
    int fd = open("server_database/messages.json", O_RDONLY);
    if(fd < 0){
        perror("Error opening messages database");
        return;
    }
    size_t file_length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* buffer = (char*)malloc((file_length + 1) * sizeof(char));
    if(buffer == NULL){
        perror("Error allocating memory for messages buffer");
        return;
    }
    if(read(fd, buffer, file_length) < 0){
        perror("Error reading from messages database");
        return;
    }
    close(fd);
    buffer[file_length] = '\0';
    struct json_object* parsed_json = json_tokener_parse(buffer);
    if(!parsed_json){
        perror("Failed to parse JSON string");
        return;
    }
    size_t array_length = json_object_array_length(parsed_json);
    for(int i = 0; i < array_length; ++i){
        struct json_object* message = json_object_array_get_idx(parsed_json, i);
        Message* msg = create_Message_From_Json(message);
        if(msg){
            store_Message(server, msg);
        }else{
            perror("Empty message. Can't store it");
        }
    }
    free(buffer);
    json_object_put(parsed_json);
}

void save_messages_in_database(Server* server){
    printf("\nMessages will be saved in server_database/messages.json\n");
    int fd = open("server_database/messages.json", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0){
        perror("Error opening messages database");
        return;
    }
    char* json_string = serialize_Hashtable(server->messages);
    if(json_string == NULL){
        perror("Error serializing messages");
        return;
    }
    size_t json_length = strlen(json_string);
    if(write(fd, json_string, json_length) < 0){
        perror("Error writing to messages database");
        return;
    }
    close(fd);
    free(json_string);
}


void sigint_handler(int sig) {
    atomic_store(&should_terminate, true);
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

void init_TaskQueue(TaskQueue* queue) {
    queue->front = 0;
    queue->rear = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void enqueue_TaskQueue(TaskQueue* queue, int client_fd) {
    pthread_mutex_lock(&queue->mutex);
    if ((queue->rear + 1) % MAX_EVENTS == queue->front) {
        printf("Task queue full! Client FD %d was not enqueued.\n", client_fd);
    } else {
        queue->client_fd_queue[queue->rear] = client_fd;
        queue->rear = (queue->rear + 1) % MAX_EVENTS;
        pthread_cond_signal(&queue->cond);
    }
    pthread_mutex_unlock(&queue->mutex);
}


int dequeue_TaskQueue(TaskQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->front == queue->rear && !atomic_load(&should_terminate)) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    if (atomic_load(&should_terminate)) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;  // Signal for termination or error
    }
    int client_fd = queue->client_fd_queue[queue->front];
    queue->front = (queue->front + 1) % MAX_EVENTS;
    pthread_mutex_unlock(&queue->mutex);
    return client_fd;
}

void* thread_send_messages(void* sv)
{
    Server* server = (struct Server*)sv;
    while(!atomic_load(&should_terminate))
    {   
        sleep(5);                               //if value modified, you should consider verification for the termination signal (should_terminate)
        if (atomic_load(&should_terminate)) 
            break;
        send_messages_to_subs(server);
    }
    printf("Message sender thread shutting down...\n");
    return NULL;
}


void* worker_Thread(void* arg) {
    struct Server* server = (struct Server*)arg; 
    TaskQueue* queue = &(server->task_queue);   
    
    while (!atomic_load(&should_terminate)) {
        int client_fd = dequeue_TaskQueue(queue);
        
        if (client_fd == -1 || atomic_load(&should_terminate)) {
            break;
        }
        
        handle_Client_Read(server, client_fd);
    }
    printf("Worker thread shutting down...\n");
    return NULL;
}

void store_Message(Server* server, Message* msg) {
    insert_Hashtable(server->messages, msg->header.topic, msg->header.subtopic,
                     msg);
}

void store_Subscribtion(Server* server, Subscribtion* sub) {
    insert_Hashtable(server->subscribtions, sub->topic,
                     sub->subtopic, sub);
}


void handle_Publishing(Server* server, struct json_object* parsed_msg) {
    Message* msg = create_Message_From_Json(parsed_msg);
	printf("About to store a message with topic %s and subtopic %s\n\n", msg->header.topic, msg->header.subtopic);
    store_Message(server, msg);
    // print_Hashtable(server->messages);
}
void handle_Subscription(Server* server, struct json_object* parsed_msg,
                         int client_fd) {
    Subscribtion* sub = create_Subscribtion_From_Json(parsed_msg, client_fd);
    store_Subscribtion(server, sub);
    // print_Hashtable(server->subscribtions);
}
void handle_Notification(Server* server, struct json_object* parsed_msg) {}
void process_json_message(Server* server, json_object* parsed_msg, int client_fd) {
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
            handle_Subscription(server, parsed_msg, client_fd);
            break;
        case MSG_ALERT:
            handle_Notification(server, parsed_msg);
            break;
        default:
            fprintf(stderr, "Invalid message type: %d\n", msg_type);
    }
}
void parse_received_json(Server* server, char* json_string, int client_fd) {
    struct json_tokener* tok = json_tokener_new();
    struct json_object* json_msg;
    enum json_tokener_error jerr;

    char* start = json_string;
	char* end = start + strlen(start);

    while (start < end) {
        json_msg = json_tokener_parse_ex(tok, start, end - start);
        jerr = json_tokener_get_error(tok);
        if (jerr == json_tokener_success) {
			printf("Received JSON message: %.*s\n", (int)tok->char_offset, start);
            process_json_message(server, json_msg, client_fd);

            start += (tok->char_offset);
            json_object_put(json_msg);

			json_tokener_reset(tok);
        } else if (jerr == json_tokener_continue) {
            break;
        } else {
            fprintf(stderr, "JSON parsing error: %s\n", json_tokener_error_desc(jerr));
            ++start;
			json_tokener_reset(tok);
        }
    }
    size_t remaining = end - start;
    memmove(json_string, start, remaining);
    json_string[remaining] = '\0';

    json_tokener_free(tok);
}

void handle_Client_Read(Server* server, int client_fd) {
    char buf[BUFSIZ / 2];
    while (1) {
        int bytesRead = read(client_fd, buf, sizeof(buf));
        if (bytesRead == 0) {
            printf("Client disconnected from server\n");
            pthread_mutex_lock(&client_queue_mutex);
            Client* client = return_Client_from_Queue(&clients_fd_queue, client_fd);
            if(client != NULL){
                free(client);
            }
            pthread_mutex_unlock(&client_queue_mutex);
            epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL); 
            close(client_fd);
            break;
        } else if (bytesRead < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                Client* client = return_Client_from_Queue(&clients_fd_queue, client_fd);
                if(client != NULL){
                    free(client);
                }
                epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL); 
                close(client_fd);
                break;
            }
        } else {
            buf[bytesRead] = '\0';
            printf("Received: %d bytes\n", bytesRead);
			printf("The data: %s\n\n", buf);
            parse_received_json(server, buf, client_fd);
        }
    }
}
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
Server* init_Server(in_addr_t _addr, int _port) {
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
    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        close(server->server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Socket created succesfully\n");

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_port = htons(_port);
    server->server_addr.sin_addr.s_addr = _addr;

    server->messages = create_Hashtable();
    server->subscribtions = create_Hashtable();

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

    load_messages_from_database(server);
    init_TaskQueue(&(server->task_queue));
    return server;
}

void *debug_print(void* arg)
{
    sleep(4);
    Server* server = (struct Server*)arg;

    printf("\n\nDebug print:\n");
    print_Hashtable(server->subscribtions);
}

void close_Server(Server* server) {
    write(STDOUT_FILENO, "\nCaught Ctrl+C. Initiating shutdown...\n", 39);

    pthread_mutex_lock(&global_server->task_queue.mutex);
    pthread_cond_broadcast(&global_server->task_queue.cond);
    pthread_mutex_unlock(&global_server->task_queue.mutex);

    for(int i = 0; i < WORKER_THREADS; i++){
        pthread_join(recv_thread[i], NULL);
    }
    pthread_join(thread_send_msg, NULL);

    save_messages_in_database(server);

    pthread_mutex_destroy(&server->task_queue.mutex);
    pthread_cond_destroy(&server->task_queue.cond);
    
    close(server->epoll_fd);
    close(server->server_fd);

    free_Queue(&clients_fd_queue);

    free_Hashtable(server->subscribtions);
    free_Hashtable(server->messages);

    free(server);
}

void start_Epoll_Server(Server* server) {
    server->epoll_fd = epoll_create1(0);
    if (server->epoll_fd == -1) {
        perror("Error creating epoll instance");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server->server_fd;

    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->server_fd, &ev) ==
        -1) {
        perror("Error adding server socket to epoll");
        exit(EXIT_FAILURE);
    }

    set_Non_Blocking(server->server_fd);

    for(int i = 0; i < WORKER_THREADS; i++){
        if (pthread_create(&recv_thread[i], NULL, worker_Thread, server) != 0) {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }
    
    if(pthread_create(&thread_send_msg, NULL, thread_send_messages, server) != 0){
        perror("Error creating send message thread");
        exit(EXIT_FAILURE);
    }

    // pthread_t debug_thread;
    // pthread_create(&debug_thread, NULL, debug_print, server);

    while (atomic_load(&should_terminate) == false) {
        int nfds = epoll_wait(server->epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Error during epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server->server_fd) {
                while (1) {
                    Client* client = (Client*)malloc(sizeof(Client));
                    if (!client) {
                        perror("Error allocating client memory");
                        continue;
                    }

                    unsigned int client_size = sizeof(client->client_addr);
                    client->client_fd = accept(
                        server->server_fd,
                        (struct sockaddr*)&client->client_addr, &(client_size));
                    if (client->client_fd < 0) {
                        free(client);
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;  // All connections processed
                        } else {
                            perror("Can't accept connection");
                        }
                        continue;
                    }

                    printf("New client connected: FD=%d\n", client->client_fd);
                    set_Non_Blocking(client->client_fd);

                    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                    ev.data.fd = client->client_fd;

                    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD,
                                  client->client_fd, &ev) == -1) {
                        perror("Error adding client socket to epoll");
                        close(client->client_fd);
                        free(client);
                        continue;
                    }
                    push_Queue(&clients_fd_queue, (void*)client);
                }
            } else {
                int client_fd = events[i].data.fd;
                if (events[i].events & (EPOLLIN)) {
                    enqueue_TaskQueue(&(server->task_queue), client_fd); 
                }
            }
        }
    }

    close_Server(global_server);
}





Message* retrieve_Message(Server* server, const char* topic,
                          const char* subtopic) {
    unsigned int index = hash_Function(topic);
    HashTableEntry* entry = server->messages->buckets[index];
    HashTableEntry* prev = NULL;
    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            Queue_Node* messages = search_Rbt(entry->tree, subtopic);
            if (messages) {
                void* data = pop_Queue(&messages);
                if (messages == NULL) {
                    delete_Rbt(entry->tree, subtopic);
                }
                if (entry->tree->root == entry->tree->NIL) {
                    if (prev) {
                        prev->next = entry->next;
                    } else {
                        server->messages->buckets[index] = entry->next;
                    }
                    free_Rbt(entry->tree);
                    free(entry);
                }
                return (Message*)data;
            }
            fprintf(stderr, "Subtopic '%s' is not found under topic '%s'\n",
                    subtopic, topic);
            return NULL;
        }
        prev = entry;
        entry = entry->next;
    }
    fprintf(stderr, "Topic '%s' not found in the hash table\n", topic);
    return NULL;
}
int main() {
    setbuf(stdout, NULL);
    Server* server = init_Server(INADDR_ANY, SERVER_PORT);
    global_server = server;
    setup_signal_handler();
    start_Epoll_Server(server);

    return 0;
}
