#include "../header/rbtree.h"

RBTNode* create_Rbt_Node(Message* msg) {
    RBTNode* node = (RBTNode*)malloc(sizeof(RBTNode));
    if (node == NULL) {
        perror("Error allocating memory for node");
        exit(-1);
    }
    node->msg = msg;
    node->color = RED;
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    return node;
}
RBTree* create_Rbtree() {
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    if (tree == NULL) {
        perror("Error allocating message for rbtree");
        exit(EXIT_FAILURE);
    }
	Message* tmp_msg = (Message*)malloc(sizeof(Message));
	memset(tmp_msg->header.topic, 0, TOPICSIZ);
	tmp_msg->data = NULL;

	tree->NIL = create_Rbt_Node(tmp_msg);
    tree->NIL->color = BLACK;
    tree->root = tree->NIL;
    return tree;
}
void left_Rbt_Rotate(RBTree* tree, RBTNode* x) {
    RBTNode* y = x->right;
    x->right = y->left;

    if (y->left != tree->NIL)
        y->left->parent = x;
    y->parent = x->parent;

    if (x->parent == tree->NIL)
        tree->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;
}
void right_Rbt_Rotate(RBTree* tree, RBTNode* y) {
    RBTNode* x = y->left;
    y->left = x->right;

    if (x->right != tree->NIL)
        x->right->parent = y;

    x->parent = y->parent;

    if (y->parent == tree->NIL)
        tree->root = x;
    else if (y == y->parent->right)
        y->parent->right = x;
    else
        y->parent->left = x;

    x->right = y;
    y->parent = x;
}
void insert_Rbt_Balance(RBTree* tree, RBTNode* z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBTNode* y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color = BLACK;
                z->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_Rbt_Rotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                right_Rbt_Rotate(tree, z->parent->parent);
            }
        } else {
            RBTNode* y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_Rbt_Rotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                left_Rbt_Rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
}
void insert_Rbt(RBTree* tree, Message* msg) {
    RBTNode* z = create_Rbt_Node(msg);
    RBTNode* y = tree->NIL;
    RBTNode* x = tree->root;

    while (x != tree->NIL) {
        y = x;
        if (z->msg < x->msg)
            x = x->left;
        else
            x = x->right;
    }

    z->parent = y;
    if (y == tree->NIL)
        tree->root = z;
    else if (z->msg < y->msg)
        y->left = z;
    else
        y->right = z;

    z->left = tree->NIL;
    z->right = tree->NIL;
    z->color = RED;

    insert_Rbt_Balance(tree, z);
}
RBTNode* search_Rbt(RBTree* tree, RBTNode* node, const char* subtopic){
	if(node == tree->NIL || strcmp(node->msg->header.topic, subtopic)==0)
		return node;
	if(strcmp(subtopic, node->msg->header.topic)<0)
		return search_Rbt(tree, node->left, subtopic);
	else
		return search_Rbt(tree, node->right, subtopic);
}
void transplant_Rbt(RBTree* tree, RBTNode* u, RBTNode* v) {
    if (u->parent == tree->NIL)
        tree->root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    v->parent = u->parent;
}
RBTNode* minimum_Rbt(RBTree* tree, RBTNode* x) {
    while (x->left != tree->NIL)
        x = x->left;
    return x;
}
void delete_Rbt_Balance(RBTree* tree, RBTNode* x) {
    while (x != tree->root && x->color == BLACK) {
        if (x == x->parent->left) {
            RBTNode* w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                left_Rbt_Rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    right_Rbt_Rotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                left_Rbt_Rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            RBTNode* w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                right_Rbt_Rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    left_Rbt_Rotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                right_Rbt_Rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = BLACK;
}
void delete_Rbt(RBTree *tree, RBTNode *z){
	RBTNode* y=z;
	RBTNode* x;
	Color y_original_color = y->color;

	if(z->left==tree->NIL){
		x=z->right;
		transplant_Rbt(tree, z, z->right);
	}else if(z->right==tree->NIL){
		x=z->left;
		transplant_Rbt(tree, z, z->left);
	}else{
		y=minimum_Rbt(tree, z->right);
		y_original_color = y->color;
		x=y->right;

		if(y->parent==z)
			x->parent=y;
		else{
			transplant_Rbt(tree, y, y->right);
			y->right=z->right;
			y->right->parent=y;
		}

		transplant_Rbt(tree, z, y);
		y->left=z->left;
		y->left->parent=y;
		y->color=z->color;
	}

	if(y_original_color==BLACK)
		delete_Rbt_Balance(tree, x);

	free(z);
}
void free_Rbt(RBTree* tree){
	RBTNode* current=tree->root;
	RBTNode* prev=NULL;

	while(current!=tree->NIL){
		if(current->left==tree->NIL){
			prev=current;
			current=current->right;
			free(prev);
		}else{
			RBTNode* predecessor=current->left;
			while(predecessor->right!=tree->NIL && predecessor->right!=current){
				predecessor=predecessor->right;
			}

			if(predecessor->right==tree->NIL){
				predecessor->right=current;
				current=current->left;
			}else{
				predecessor->right=tree->NIL;
				prev=current;
				current=current->right;
				free(prev);
			}
		}
	}
	free(tree->NIL);
	free(tree);
}
