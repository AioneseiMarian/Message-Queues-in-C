#ifndef QUEUES_H
#define QUEUES_H

#include <stdio.h>
#include <stdlib.h>
#include "message.h"

#define INITIAL_QUEUE_SIZE 4


typedef struct Heap{
    Message* heap;
    int capacity;
    int size;
}Heap;


//Subscriber
typedef struct RequestQueue{
    struct RequestQueue* next_node;
    Request* request;
}RequestQueue;


RequestQueue* createReqQueue();
void pushRequest(RequestQueue* _req_queue, Request* _request);
Request* popRequest(RequestQueue* _req_queue);
void freeReqQueue(RequestQueue* _req_queue);
//endSubscriber

void swapMsgs(Message* _msg1, Message* _msg2);
void heapifyUp(Heap* _heap, int _index);
void heapifyDown(Heap* _heap, int _index);
void pushHeap(Heap* _heap, Message _msg);
Message popHeap(Heap* _heap);
Heap* createQueue(int _capacity);
void resizeHeap(Heap* _heap);
void freeQueue(Heap* _heap);

#endif