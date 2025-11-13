#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"

//
// Here, you have to write (almost) ALL the code. Oh no!
// How can you show that a thread does not starve
// when attempting to acquire this mutex you build?
//

typedef struct __ns_mutex_t {
    sem_t mutex;      // Protects the queue counter
    sem_t next;       // Queue for waiting threads (FIFO)
    int waiting;      // Number of threads waiting in queue
} ns_mutex_t;

int counter = 0;  // Shared counter to test mutex
ns_mutex_t m;

void ns_mutex_init(ns_mutex_t *m) {
    m->waiting = 0;
    sem_init(&m->mutex, 0, 1);   // Binary semaphore for mutual exclusion
    sem_init(&m->next, 0, 0);    // Queue starts empty
}

void ns_mutex_acquire(ns_mutex_t *m) {
    sem_wait(&m->mutex);         // Enter critical section
    
    if (m->waiting > 0) {
        // There are threads waiting, join the queue
        m->waiting++;
        sem_post(&m->mutex);     // Release mutex before blocking
        sem_wait(&m->next);      // Wait in FIFO queue
    } else {
        // No one waiting, proceed immediately
        sem_post(&m->mutex);     // Release mutex
    }
}

void ns_mutex_release(ns_mutex_t *m) {
    sem_wait(&m->mutex);         // Enter critical section
    
    if (m->waiting > 0) {
        // Wake up next thread in queue
        m->waiting--;
        sem_post(&m->next);      // Signal next waiting thread
    }
    
    sem_post(&m->mutex);         // Release mutex
}

void *worker(void *arg) {
    int thread_id = *(int *)arg;
    int i;
    
    for (i = 0; i < 5; i++) {
        ns_mutex_acquire(&m);
        
        // Critical section
        printf("Thread %d acquired mutex (counter = %d)\n", thread_id, counter);
        counter++;
        sleep(1);  // Simulate work to make starvation visible
        printf("Thread %d releasing mutex (counter = %d)\n", thread_id, counter);
        
        ns_mutex_release(&m);
        
        // Small delay before trying again
        usleep(100000);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("parent: begin\n");
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }
    
    int num_threads = atoi(argv[1]);
    pthread_t threads[num_threads];
    int thread_ids[num_threads];
    
    ns_mutex_init(&m);
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        Pthread_create(&threads[i], NULL, worker, &thread_ids[i]);
    }
    
    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {
        Pthread_join(threads[i], NULL);
    }
    
    printf("Final counter value: %d (expected: %d)\n", counter, num_threads * 5);
    printf("parent: end\n");
    return 0;
}