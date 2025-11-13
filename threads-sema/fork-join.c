#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"

sem_t s; 

void *child(void *arg) {
    printf("child\n");
    sleep(1);  // Added to ensure it's working (cause parent to wait)
    sem_post(&s);  // Signal that child is done
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    printf("parent: begin\n");
    sem_init(&s, 0, 0);  // Initialize semaphore to 0
    Pthread_create(&p, NULL, child, NULL);
    sem_wait(&s);  // Wait for child to signal completion
    printf("parent: end\n");
    return 0;
}

