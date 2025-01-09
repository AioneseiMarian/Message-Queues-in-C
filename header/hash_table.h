#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "rbtree.h"
#include <pthread.h>
#define HASH_TABLE_SIZE 128

typedef struct HashTableEntry{
	char topic[TOPICSIZ];
	RBTree* tree;
	struct HashTableEntry* next;
}HashTableEntry;

typedef struct HashTable{
	HashTableEntry* buckets[HASH_TABLE_SIZE];
}HashTable;



HashTable* create_Hashtable();
unsigned int hash_Function(const char* key);
RBTree* search_Hashtable(HashTable* table, const char* topic);
void insert_Hashtable(HashTable* table, const char* topic, const char* subtopic, void* data);
void delete_Hashtable(HashTable* table, const char* topic);
void free_Hashtable(HashTable* table);
void print_Hashtable(HashTable* table);
Queue_Node* get_Queue(HashTable* table, const char* topic, const char* subtopic);
char* serialize_Hashtable(HashTable* hashtable);

#endif 
