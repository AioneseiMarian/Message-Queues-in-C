#ifndef QUEUES_H
#define QUEUES_H

#include <stdio.h>
#include <stdlib.h>
#include "message.h"

typedef struct Heap{
    Message* heap;
    int capacity;
    int size;
}Heap;

void swapMsgs(Message* _msg1, Message* _msg2);
void heapifyUp(Heap* _heap, int _index);
void heapifyDown(Heap* _heap, int _index);
<<<<<<< HEAD
void push(Heap* _heap, Message _msg);
Message popHead(Heap* _heap);
Heap* create_queue(int _capacity);
=======
void pushHeap(Heap* _heap, Message _msg);
Message popHeap(Heap* _heap);
Heap* createQueue(int _capacity);
void resizeHeap(Heap* _heap);
>>>>>>> 81e2ad4a88e510a39d8fc563ba942c3b7e217c6b
void freeQueue(Heap* _heap);

#endif