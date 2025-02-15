#include "../header/hash_table.h"

pthread_mutex_t table_mutex;

HashTable* create_Hashtable() {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) {
        perror("Error allocating memory for Hash Table");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}
unsigned int hash_Function(const char* key) {
    unsigned int hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}
RBTree* search_Hashtable(HashTable* table, const char* topic) {
    unsigned int index = hash_Function(topic);
    HashTableEntry* entry = table->buckets[index];
    pthread_mutex_lock(&table_mutex);
    printf("Mutex locked in %s\n", __func__);

    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            pthread_mutex_unlock(&table_mutex);
            printf("Mutex unlocked in %s\n", __func__);
            return entry->tree;
        }
        entry = entry->next;
    }
    pthread_mutex_unlock(&table_mutex);
    printf("Mutex unlocked in %s\n", __func__);
    return NULL;
}
void insert_Hashtable(HashTable* table, const char* topic, const char* subtopic,
                      void* data) {
    unsigned int index = hash_Function(topic);
    HashTableEntry* entry = table->buckets[index];
    HashTableEntry* prev = NULL;

    pthread_mutex_lock(&table_mutex);
    printf("Mutex locked in %s\n", __func__);
    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            insert_Rbt(entry->tree, subtopic, data);
            pthread_mutex_unlock(&table_mutex);
            printf("Mutex unlocked in %s\n", __func__);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
    HashTableEntry* new_entry = (HashTableEntry*)malloc(sizeof(HashTableEntry));
    if (!new_entry) {
        perror("Error allocating memory for Hash Table Entry");
        exit(EXIT_FAILURE);
    }
    strcpy(new_entry->topic, topic);
    new_entry->tree = create_Rbtree();
    insert_Rbt(new_entry->tree, subtopic, data);
    new_entry->next = NULL;
    if (prev) {
        prev->next = new_entry;
    } else {
        table->buckets[index] = new_entry;
    }
    pthread_mutex_unlock(&table_mutex);
    printf("Mutex unlocked in %s\n", __func__);
}
void delete_Hashtable(HashTable* table, const char* topic) {
    unsigned int index = hash_Function(topic);
    HashTableEntry* entry = table->buckets[index];
    HashTableEntry* prev = NULL;

    pthread_mutex_lock(&table_mutex);
    printf("Mutex locked in %s\n", __func__);
    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                table->buckets[index] = entry->next;
            }
            free_Rbt(entry->tree);
            free(entry);
            /* printf("Topic '%s' deleted from the hash table.\n", topic); */
            pthread_mutex_unlock(&table_mutex);
            printf("Mutex unlocked in %s\n", __func__);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
    pthread_mutex_unlock(&table_mutex);
    printf("Mutex unlocked in %s\n", __func__);
    printf("Topic '%s' is not found in the hash table.\n", topic);
}
void free_Hashtable(HashTable* table) {
    pthread_mutex_lock(&table_mutex);
    printf("Mutex locked in %s\n", __func__);
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashTableEntry* entry = table->buckets[i];
        while (entry) {
            HashTableEntry* temp = entry;
            entry = entry->next;
            free_Rbt(temp->tree);
            free(temp);
        }
    }
    free(table);
    pthread_mutex_unlock(&table_mutex);
    printf("Mutex unlocked in %s\n", __func__);
}
void print_Hashtable(HashTable* table) {
    pthread_mutex_lock(&table_mutex);
    printf("Mutex locked in %s\n", __func__);
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashTableEntry* entry = table->buckets[i];
        if (entry) {
            printf("Bucket %d:\n", i);
            while (entry) {
                printf("\tTopic: %s\n", entry->topic);
                print_Rbt_inorder(entry->tree, entry->tree->root);
                entry = entry->next;
            }
        }
    }
    pthread_mutex_unlock(&table_mutex);
    printf("Mutex unlocked in %s\n", __func__);
}
Queue_Node* get_Queue(HashTable* table, const char* topic, const char* subtopic) {
    unsigned int index = hash_Function(topic);
    HashTableEntry* entry = table->buckets[index];

    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            Queue_Node* queue = search_Rbt(entry->tree, subtopic);
            return queue;
        }
        entry = entry->next;
    }
    return NULL;
}

char* serialize_Hashtable(HashTable* hashtable){
    json_object* json_array = json_object_new_array();

    for(int i = 0; i < HASH_TABLE_SIZE; i++){
        HashTableEntry* entry = hashtable->buckets[i];
        while(entry){
                Queue_Node* aux_queue = NULL;  // Initialize to NULL as per push_Queue's expectation
                push_Queue(&aux_queue, (void*)entry->tree->root);

                while (aux_queue != NULL) {  // While queue is not empty
                    RBTNode* tree_node = (RBTNode*)pop_Queue(&aux_queue);
                    if (tree_node && tree_node->queue) {
                        //iterate the queue
                        Queue_Node* msg_queue = tree_node->queue;
                        while(msg_queue != NULL)
                        {
                            Message* msg = msg_queue->data;
                            json_object* json_message = json_object_new_object();
                            json_object_object_add(json_message, "type", json_object_new_int(msg->header.msg_type));
                            json_object_object_add(json_message, "topic", json_object_new_string(msg->header.topic));
                            json_object_object_add(json_message, "subtopic", json_object_new_string(msg->header.subtopic));
                            json_object_object_add(json_message, "length", json_object_new_int(msg->header.len));
                            json_object_object_add(json_message, "data", json_object_new_string(msg->data));
                            json_object_array_add(json_array, json_message);
                            msg_queue = msg_queue->next_node;

                        }
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

    const char* json_string = json_object_to_json_string(json_array);
    printf("Serialized message: %s\n", json_string);
    char *string = strdup(json_string);
    json_object_put(json_array);

    return string;
}
