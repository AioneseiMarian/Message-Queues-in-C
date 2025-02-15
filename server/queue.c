#include "../header/queues.h"
#include "../header/server.h"

Queue_Node* create_Queue_Node() {
    Queue_Node* node = (Queue_Node*)malloc(sizeof(Queue_Node));
    if (!node) {
        perror("Error allocating queue memory");
        return NULL;
    }
    node->next_node = NULL;
    node->data = NULL;
    return node;
}
void push_Queue(Queue_Node** head, void* data) {
    if (!head) {
        fprintf(stderr, "Queue head is still NULL. Can't allocate data");
        return;
    }
    Queue_Node* node = create_Queue_Node();
    if (!node) {
        return;
    }
    node->data = data;
    if (*head == NULL) {
        *head = node;
    } else {
        Queue_Node* current = *head;   
        while (current->next_node != NULL) {
            current = current->next_node;
        }
        current->next_node = node;
    }
}
void* pop_Queue(Queue_Node** head) {
    if (!head || !*head) {
        fprintf(stderr, "Queue is empty or NULL. Can't pop.\n");
        return NULL;
    }
    Queue_Node* node = *head;
    void* data = node->data;
    *head = node->next_node;
    free(node);
    return data;
}
void free_Queue(Queue_Node** head) {
    if (!head) {
        fprintf(stderr, "Queue head is NULL. Nothing to free.\n");
        return;
    }
    Queue_Node* current = *head;
    while (current != NULL) {
        Queue_Node* next_node = current->next_node;
        free(current);
        current = next_node;
    }
    *head = NULL;
}
int get_Queue_Size(Queue_Node* head) {
    int size = 0;
    Queue_Node* current = head;
    while (current != NULL) {
        size++;
        current = current->next_node;
    }
    return size;
}

Client* return_Client_from_Queue(Queue_Node** head, int client_fd) {
    Queue_Node* current = *head;
    Queue_Node* prev = NULL;
    while (current != NULL) {
        Client* client = (Client*)current->data;
        if (client->client_fd == client_fd) {
            if (prev == NULL) {
                *head = current->next_node;
            } else {
                prev->next_node = current->next_node;
            }
            return client;
        }
        current = current->next_node;
        prev = current;
    }
    return NULL;
}