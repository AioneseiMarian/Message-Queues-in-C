#ifndef QUEUES_H
#define QUEUES_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Queue_Node{
	struct Queue_Node* next_node;
	void* data;
}Queue_Node;

Queue_Node* create_Queue_Node();
void push_Queue(Queue_Node** head, void* data);
void* pop_Queue(Queue_Node** head);
void free_Queue(Queue_Node** head);
int get_Queue_Size(Queue_Node* head);

struct Client;

struct Client* return_Client_from_Queue(Queue_Node* head, int client_fd); //used ONLY for closing all connections at shutdown

#endif
