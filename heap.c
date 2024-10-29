#include "queues.h"

void swap(Message* _msg1, Message* _msg2){
    Message tmp = *_msg1;
    *_msg1 = *_msg2;
    *_msg2 = tmp;
}
void heapifyUp(Heap* _heap, int _index){
    int parent = (_index - 1)/2;
    while(_index > 0 && _heap->heap[_index].has_prioriry > _heap->heap[parent].has_prioriry){
        swap(&_heap->heap[_index] ,&_heap->heap[parent]);
        _index = parent;
        parent = (_index - 1)/2;
    }
}
void heapifyDown(Heap* _heap, int _index){
    int left = 2 * _index + 1;
    int right = left + 1;
    int largest = _index;

    if(left < _heap->size && _heap->heap[left].has_prioriry > _heap->heap[largest].has_prioriry){
        largest = left;
    }
    if(right < _heap->size && _heap->heap[right].has_prioriry > _heap->heap[largest].has_prioriry){
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
    
}
