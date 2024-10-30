#ifndef QUEUES_H
#define QUEUES_H

#include <stdlib.h>
#include "message.h"

typedef struct Heap{
    Message* heap;
    int capacity;
    int size;
}Heap;

void swap(Message* _msg1, Message* _msg2);
void heapifyUp(Heap* _heap, int _index);
void heapifyDown(Heap* _heap, int _index);
void push(Heap* _heap, Message _msg);
Message pop(Heap* _heap);
Heap* create_queue(int _capacity);
void freeQueue(Heap* _heap);

#endif