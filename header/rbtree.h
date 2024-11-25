#ifndef RBTREE_H
#define RBTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

typedef enum Color {
	RED,
	BLACK			
} Color;

typedef struct RBTNode {
	Message* msg;
	Color color;
	struct RBTNode* parent;
	struct RBTNode* left;
	struct RBTNode* right;					
} RBTNode;

typedef struct RBTree {
	RBTNode* root;
	RBTNode* NIL;
} RBTree;

RBTNode* create_Rbt_Node(Message* msg);
RBTree* create_Rbtree();
void left_Rbt_Rotate(RBTree* tree, RBTNode* x);
void right_Rbt_Rotate(RBTree* tree, RBTNode* y);
void insert_Rbt_Balance(RBTree* tree, RBTNode* z);
void insert_Rbt(RBTree* tree, Message* msg);
RBTNode* search_Rbt(RBTree* tree, RBTNode* node, const char* subtopic);
void transplant_Rbt(RBTree* tree, RBTNode* u, RBTNode* v);
RBTNode* minimum_Rbt(RBTree* tree, RBTNode* x);
void delete_Rbt_Balance(RBTree* tree, RBTNode* x);
void delete_Rbt(RBTree* tree, RBTNode* z);
void free_Rbt(RBTree* tree);

#endif
