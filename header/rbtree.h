#ifndef RBTREE_H
#define RBTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "queues.h"

#define RED 1
#define BLACK 0

typedef struct RBTNode {
	char subtopic[SUBTOPICSIZ];
	Queue_Node* messages;
	int color;
	struct RBTNode* parent;
	struct RBTNode* left;
	struct RBTNode* right;					
} RBTNode;

typedef struct RBTree {
	RBTNode* root;
	RBTNode* NIL;
} RBTree;

RBTree* create_Rbtree();
void insert_Rbt(RBTree* tree, const char* subtopic, void* data);
Queue_Node* search_Rbt(RBTree* tree, const char* subtopic);
void delete_Rbt(RBTree* tree, const char* subtopic);
void free_Rbt(RBTree* tree);
void print_Rbt_inorder(RBTree* tree, RBTNode* node);
#endif
