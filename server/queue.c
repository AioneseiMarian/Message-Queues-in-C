#include "../header/queues.h"
#include <string.h>

RequestNode* createReqNode(){
    RequestNode* node = (RequestNode*) malloc(sizeof(RequestNode));
    if (node == NULL) {
        perror("Error allocating memory");
        exit(-1);
    }
    node->next_node = NULL;
    node->request = NULL;
    return node;
}


RequestQueue* createReqQueue(char channel[40]) {
    RequestQueue* req_queue = (RequestQueue*)malloc(sizeof(RequestQueue));
    if (req_queue == NULL) {
        perror("Error allocating memory");
        exit(-1);
    }
    req_queue->head_node = createReqNode();
    strncpy(req_queue->chanell_type, channel, sizeof(req_queue->chanell_type) - 1);
    req_queue->chanell_type[sizeof(req_queue->chanell_type) - 1] = '\0'; 

    return req_queue;
}
void pushRequest(RequestQueue** _req_queue, Request* _request, char channel[40]) {
    if (*_req_queue == NULL) {
        *_req_queue = createReqQueue(channel);
        (*_req_queue)->head_node->request = _request;
        return;
    }

    RequestNode* currentNode = (*_req_queue)->head_node;
    while (currentNode->next_node != NULL) {
        currentNode = currentNode->next_node;
    }
    currentNode->next_node = createReqNode();
    currentNode->next_node->request = _request;
}


Request* popRequest(RequestQueue* _req_queue) {
    if (_req_queue == NULL || _req_queue->head_node == NULL) {
        fprintf(stderr, "Trying to pop from empty Request Queue");
        return NULL;
    }

    RequestNode* aux = _req_queue->head_node;
    _req_queue->head_node = _req_queue->head_node->next_node;

    Request* poped_request = aux->request;
    free(aux);
    aux = NULL;

    if(_req_queue->head_node == NULL){
        free(_req_queue);
        _req_queue == NULL;
    }

    return poped_request;
}

void freeReqQueue(RequestQueue** _req_queue) {
    if (*_req_queue == NULL) {
        return;
    }

    RequestNode* current_node = (*_req_queue)->head_node;
    while (current_node != NULL) {
        RequestNode* next_node = current_node->next_node;
        free(current_node->request);
        free(current_node);
        current_node = next_node;
    }

    free(*_req_queue);
    *_req_queue = NULL; 
}

