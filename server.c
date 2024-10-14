#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* print_message(void* arg) {
    char* message = (char*) arg;
    printf("%s\n", message);
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    char* msg = "Hello from the thread!";
    int ret = pthread_create(&thread, NULL, print_message, (void*) msg);
    if (ret) {
        fprintf(stderr, "Error creating thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_join(thread, NULL); // Wait for the thread to finish
    return 0;
}