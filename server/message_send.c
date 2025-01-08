#include "../header/server.h"
#include "../header/message_send.h"


void set_all_messages_as_old(Queue_Node* msg_queue){  //From a specific topic and subtopic
    Message* msg;
    while(msg_queue != NULL)
    {
        msg = msg_queue->data;
        msg->header.new = 0;

        msg_queue = msg_queue->next_node;
    }
}

void set_all_subs_as_old(HashTable* table){
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashTableEntry* entry = table->buckets[i];
        while (entry) {
            Queue_Node* sub_queue = entry->tree->root->queue;
            while(sub_queue != NULL)
            {
                Subscribtion* sub = sub_queue->data;
                sub->new = 0;
                sub_queue = sub_queue->next_node;
            }
            entry = entry->next;
       }
    }
}


void send_message(int client_fd, const char* json_string)
{
    size_t json_length = strlen(json_string);
    ssize_t sent_bytes = send(client_fd, json_string, json_length, 0);
    if(sent_bytes < 0){
        perror("Sending message to subscriber Failed");
    }else{
        printf("Sent %zd bytes successfully.\n", sent_bytes);
    }
}

void aux_send_messages_from_queues(Server* server, Queue_Node* msg_queue){      //from a msg queue with specific topic and subtopic
    Message* msg;
    while(msg_queue != NULL)
    {
        msg = msg_queue->data;
        json_object* json_message = create_Json_From_Message(msg->header.msg_type, msg->header.topic, msg->header.subtopic, msg->header.len, msg->data);
        const char* json_string = json_object_to_json_string(json_message);

        printf("Sending message with topic %s and subtopic %s\n", msg->header.topic, msg->header.subtopic);
        Queue_Node* sub_queue = get_Queue(server->subscribtions, msg->header.topic, msg->header.subtopic);
        if(sub_queue == NULL){
            printf("No subscribers for this subtopic\n");
            return;
        }
        while(sub_queue != NULL)
        {
            Subscribtion* sub = sub_queue->data;
            if(sub->new == 0){
                if(msg->header.new == 1){
                    if(sub->has_interest == 1){
                        if(strstr(msg->data, sub->interest) != NULL){
                            send_message(sub->client_fd, json_string);
                        }
                    }else{
                        send_message(sub->client_fd, json_string);
                    }
                }
            }
            else{
                if(sub->has_interest == 1){
                    if(strstr(msg->data, sub->interest) != NULL){
                        send_message(sub->client_fd, json_string);
                    }
                }else{
                    send_message(sub->client_fd, json_string);
                }
            }

            sub_queue = sub_queue->next_node;
        }


        msg_queue = msg_queue->next_node;
    }
    set_all_messages_as_old(msg_queue);     //from this subtopic
}

void send_messages_to_subs(Server* server){
    pthread_mutex_lock(&table_mutex);

    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashTableEntry* entry = server->messages->buckets[i];
        if (entry) {
            printf("Sending bucket %d:\n", i);
            while (entry) {
                printf("\tSending topic: %s\n", entry->topic);

                Queue_Node* aux_queue = NULL;  // Initialize to NULL as per push_Queue's expectation
                push_Queue(&aux_queue, (void*)entry->tree->root);

                while (aux_queue != NULL) {  // While queue is not empty
                    RBTNode* tree_node = (RBTNode*)pop_Queue(&aux_queue);
                    if (tree_node && tree_node->queue) {
                        aux_send_messages_from_queues(server, tree_node->queue);    //send messages from rbtree node(queue)
                    }

                    if(tree_node->left != entry->tree->NIL)
                        push_Queue(&aux_queue, tree_node->left);
                    
                    if(tree_node->right != entry->tree->NIL)
                        push_Queue(&aux_queue, tree_node->right);
                }
                free_Queue(&aux_queue);
                entry = entry->next;
            }
        }
    }
    

    pthread_mutex_unlock(&table_mutex);
}