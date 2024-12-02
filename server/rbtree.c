#include "../header/rbtree.h"

RBTNode* create_Rbt_Node(RBTree* tree, const char* subtopic, void* data) {
    RBTNode* node = (RBTNode*)malloc(sizeof(RBTNode));
    if (node == NULL) {
        perror("Error allocating memory for node");
        exit(-1);
    }
    strcpy(node->subtopic, subtopic);
    node->messages = NULL;
    push_Queue(&(node->messages), data);
    node->color = RED;
    node->parent = node->left = node->right = tree->NIL;
    return node;
}
RBTree* create_Rbtree() {
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    if (tree == NULL) {
        perror("Error allocating message for rbtree");
        exit(EXIT_FAILURE);
    }
    tree->NIL = (RBTNode*)malloc(sizeof(RBTNode));
    if (tree->NIL == NULL) {
        perror("Failed to allocate memory for Red-Black Tree Node");
        exit(EXIT_FAILURE);
    }
    tree->NIL->color = BLACK;
    tree->NIL->left = tree->NIL->right = tree->NIL->parent = NULL;
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
void insert_Rbt(RBTree* tree, const char* subtopic, void* data) {
    RBTNode* z = create_Rbt_Node(tree, subtopic, data);
    RBTNode* y = tree->NIL;
    RBTNode* x = tree->root;

    while (x != tree->NIL) {
        y = x;
        if (strcmp(z->subtopic, x->subtopic) < 0)
            x = x->left;
        else if (strcmp(z->subtopic, x->subtopic) > 0)
            x = x->right;
        else {
            push_Queue(&(x->messages), data);
            free(z);
            return;
        }
    }

    z->parent = y;
    if (y == tree->NIL)
        tree->root = z;
    else if (strcmp(z->subtopic, y->subtopic) < 0)
        y->left = z;
    else
        y->right = z;

    /* z->left = tree->NIL; */
    /* z->right = tree->NIL; */
    /* z->color = RED; */

    insert_Rbt_Balance(tree, z);
}
Queue_Node* search_Rbt(RBTree* tree, const char* subtopic) {
    RBTNode* current = tree->root;

    while (current != tree->NIL) {
        int cmp = strcmp(subtopic, current->subtopic);
        if (cmp == 0)
            return current->messages;
        else if (cmp < 0)
            current = current->left;
        else
            current = current->right;
    }
    return NULL;
}
RBTNode* search_Rbt_Node(RBTree* tree, RBTNode* node, const char* subtopic) {
    while (node != tree->NIL && strcmp(subtopic, node->subtopic) != 0) {
        if (strcmp(subtopic, node->subtopic) < 0)
            node = node->left;
        else
            node = node->right;
    }
    return node;
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
void delete_Rbt(RBTree* tree, const char* subtopic) {
    RBTNode* z = search_Rbt_Node(tree, tree->root, subtopic);
    if (z == tree->NIL) {
        fprintf(stderr, "Subtopic '%s' not found in RB tree\n", subtopic);
        return;
    }
    RBTNode* y = z;
    RBTNode* x;
    int y_original_color = y->color;

    if (z->left == tree->NIL) {
        x = z->right;
        transplant_Rbt(tree, z, z->right);
    } else if (z->right == tree->NIL) {
        x = z->left;
        transplant_Rbt(tree, z, z->left);
    } else {
        y = minimum_Rbt(tree, z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z)
            x->parent = y;
        else {
            transplant_Rbt(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant_Rbt(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == BLACK)
        delete_Rbt_Balance(tree, x);

    free(z);
}
void free_Rbt(RBTree* tree) {
    RBTNode* current = tree->root;
    RBTNode* prev = NULL;

    while (current != tree->NIL) {
        if (current->left == tree->NIL) {
            prev = current;
            current = current->right;
            free(prev);
        } else {
            RBTNode* predecessor = current->left;
            while (predecessor->right != tree->NIL && predecessor->right != current) {
                predecessor = predecessor->right;
            }

            if (predecessor->right == tree->NIL) {
                predecessor->right = current;
                current = current->left;
            } else {
                predecessor->right = tree->NIL;
                prev = current;
                current = current->right;
                free(prev);
            }
        }
    }
    free(tree->NIL);
    free(tree);
}
void print_Rbt_inorder(RBTree* tree, RBTNode* node) {
    if (node != tree->NIL) {
        print_Rbt_inorder(tree, node->left);
		printf("\t\tTopic: %s\n\tMessages: %i\n\n", node->subtopic, get_Queue_Size(node->messages));
        print_Rbt_inorder(tree, node->right);
    }
}
