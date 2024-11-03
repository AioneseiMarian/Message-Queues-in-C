#include "../header/queues.h"

<<<<<<< HEAD

void swap(Message* _msg1, Message* _msg2){
=======
void swapMsgs(Message* _msg1, Message* _msg2){
>>>>>>> 81e2ad4a88e510a39d8fc563ba942c3b7e217c6b
    Message tmp = *_msg1;
    *_msg1 = *_msg2;
    *_msg2 = tmp;
}
void heapifyUp(Heap* _heap, int _index){
    int parent = (_index - 1)/2;
    while(_index > 0 && _heap->heap[_index].header.has_prioriry > _heap->heap[parent].header.has_prioriry){
        swapMsgs(&_heap->heap[_index] ,&_heap->heap[parent]);
        _index = parent;
        parent = (_index - 1)/2;
    }
}
void heapifyDown(Heap* _heap, int _index){
    int left = 2 * _index + 1;
    int right = left + 1;
    int largest = _index;

    if(left < _heap->size && _heap->heap[left].header.has_prioriry > _heap->heap[largest].header.has_prioriry){
        largest = left;
    }
    if(right < _heap->size && _heap->heap[right].header.has_prioriry > _heap->heap[largest].header.has_prioriry){
        largest = right;
    }
    if(largest != _index){
        swapMsgs(&_heap->heap[largest], &_heap->heap[_index]);
        heapifyDown(_heap, largest);
    }
}
void pushHeap(Heap* _heap, Message _msg){
    if(_heap->size == _heap->capacity){
<<<<<<< HEAD
        _heap->capacity = 2 *_heap->capacity;
        _heap->heap = realloc(_heap->heap, _heap->capacity * sizeof(Message));
        if(_heap->heap == NULL){
            perror("Eroare realocare heap:");
            exit(-1);
        } 
=======
        resizeHeap(_heap);
>>>>>>> 81e2ad4a88e510a39d8fc563ba942c3b7e217c6b
    }
    _heap->heap[_heap->size] = _msg;
    heapifyUp(_heap, _heap->size);
    _heap->size++;
}
<<<<<<< HEAD
Message popHead(Heap* _heap){
=======
Message popHeap(Heap* _heap){
>>>>>>> 81e2ad4a88e510a39d8fc563ba942c3b7e217c6b
    if(_heap->size == 0){
        fprintf(stderr, "Empty queue");
        exit(EXIT_FAILURE);
    }
    Message max_msg = _heap->heap[0];
    _heap->heap[0] = _heap->heap[_heap->size - 1];
    _heap->size--;
    heapifyDown(_heap, 0);
    return max_msg;
}
Heap* createQueue(int _capacity){
    Heap* _heap = (Heap*)malloc(sizeof(Heap));
    if(_heap == NULL){
        perror("Error allocating queue");
        exit(-1);
    }
    _heap->heap = (Message*)malloc(_capacity * sizeof(Message));
    if(_heap->heap == NULL){
        perror("Error allocating queue");
        exit(-1);
    }
    _heap->capacity = _capacity;
    _heap->size = 0;
    return _heap;
}
void resizeHeap(Heap* _heap) {
    int new_capacity = _heap->capacity * 2;
    Message* new_heap = (Message*)realloc(_heap->heap, new_capacity * sizeof(Message));
    if (new_heap == NULL) {
        perror("Error reallocating memory");
        exit(EXIT_FAILURE);
    }
    _heap->heap = new_heap;
    _heap->capacity = new_capacity;
    printf("Heap resized to new capacity: %d\n", new_capacity);
}
void freeQueue(Heap* _heap){
    free(_heap->heap);
    free(_heap);
}
