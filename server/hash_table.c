#include "../header/hash_table.h"

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
    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            return entry->tree;
        }
        entry = entry->next;
    }
    return NULL;
}
void insert_Hashtable(HashTable* table, const char* topic, const char* subtopic, void* data) {
    unsigned int index = hash_Function(topic);
    HashTableEntry* entry = table->buckets[index];
    HashTableEntry* prev = NULL;

    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
			insert_Rbt(entry->tree, subtopic, data);
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
}
void delete_Hashtable(HashTable* table, const char* topic) {
    unsigned int index = hash_Function(topic);
    HashTableEntry* entry = table->buckets[index];
    HashTableEntry* prev = NULL;

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
            return;
        }
        prev = entry;
        entry = entry->next;
    }
    printf("Topic '%s' is not found in the hash table.\n", topic);
}
void free_Hashtable(HashTable* table) {
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
}
void print_Hashtable(HashTable* table) {
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
}
