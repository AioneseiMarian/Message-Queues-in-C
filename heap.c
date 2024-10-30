#include "queues.h"

void swap(Message* _msg1, Message* _msg2){
    Message tmp = *_msg1;
    *_msg1 = *_msg2;
    *_msg2 = tmp;
}
void heapifyUp(Heap* _heap, int _index){
    int parent = (_index - 1)/2;
    while(_index > 0 && _heap->heap[_index].header.has_prioriry > _heap->heap[parent].header.has_prioriry){
        swap(&_heap->heap[_index] ,&_heap->heap[parent]);
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
        swap(&_heap[largest], &_heap->heap[_index]);
        heapifyDown(_heap, largest);
    }
}
void push(Heap* _heap, Message _msg){
    if(_heap->size == _heap->capacity){
        return;
    }
    _heap->heap[_heap->size] = _msg;
    heapifyUp(_heap, _heap->size);
    _heap->size++;
}
Message pop(Heap* _heap){
    if(_heap->size == 0){
        exit(EXIT_FAILURE);
    }
    Message max_msg = _heap->heap[0];
    _heap->heap[0] = _heap->heap[_heap->size - 1];
    _heap->size--;
    heapifyDown(_heap, 0);
    return max_msg;
}
Heap* create_queue(int _capacity){
    Heap* _heap = (Heap*)malloc(sizeof(Heap));
    if(_heap == NULL){
        perror("Error allocating queue");
    }
    _heap->heap = (Message*)malloc(_capacity * sizeof(Message));
    if(_heap->heap == NULL){
        perror("Error allocating queue");
    }
    _heap->capacity = _capacity;
    _heap->size = 0;
    return _heap;
}
void freeQueue(Heap* _heap){
    free(_heap->heap);
    free(_heap);
}
