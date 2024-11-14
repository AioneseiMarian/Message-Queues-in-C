#include "../header/queues.h"

RequestQueue* createReqQueue() {
    RequestQueue* req_queue = (RequestQueue*)malloc(sizeof(RequestQueue));
    if (req_queue == NULL) {
        perror("Error allocating memory");
        exit(-1);
    }
    req_queue->next_node = NULL;
    printf("%s", "Helo world");

    return req_queue;
}
void pushRequest(RequestQueue* _req_queue, Request* _request) {
    if (_req_queue == NULL) {
        _req_queue = createReqQueue();
        _req_queue->request = _request;
        return;
    }
    RequestQueue* currentNode = _req_queue;
    while (currentNode->next_node != NULL) {
        currentNode = currentNode->next_node;
    }
    currentNode->next_node = createReqQueue();
    currentNode->next_node->request = _request;
}

Request* popRequest(RequestQueue* _req_queue) {
    if (_req_queue == NULL) {
        fprintf(stderr, "Trying to pop from empty Request Queue");
        exit(-1);
    }

    RequestQueue* aux = _req_queue;
    _req_queue = _req_queue->next_node;

    Request* poped_request = aux->request;
    free(aux);
    return poped_request;
}

void freeReqQueue(RequestQueue* _req_queue) {
    RequestQueue* current_node = _req_queue;
    RequestQueue* next_node = current_node->next_node;

    while (next_node) {
        free(current_node->request);
        free(current_node);
        current_node = next_node;
        next_node = next_node->next_node;
    }

    free(current_node->request);
    free(current_node);
    _req_queue = NULL;
}
