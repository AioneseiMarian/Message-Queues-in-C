#ifndef MESSAGE_SEND_H
#define MESSAGE_SEND_H


void aux_send_new_messages(int client_fd, Queue_Node* msg_queue);
void set_all_messages_as_old(Queue_Node* msg_queue);
void aux_send_all_messages(int client_fd, Queue_Node* msg_queue);
void aux_send_messages_from_queues(Server* server, Queue_Node* msg_queue);
void send_messages_to_subs(Server* server);




#endif
