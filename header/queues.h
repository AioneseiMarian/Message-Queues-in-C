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
void pushHeap(Heap* _heap, Message _msg);
Message popHeap(Heap* _heap);
Heap* createQueue(int _capacity);
void resizeHeap(Heap* _heap);
void freeQueue(Heap* _heap);

#endif