#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common_threads.h"

// If done correctly, each child should print their "before" message
// before either prints their "after" message. Test by adding sleep(1)
// calls in various locations.

// You likely need two semaphores to do this correctly, and some
// other integers to track things.

typedef struct __barrier_t {
    int num_threads;       // Total number of threads
    int count;             // Counter for threads that have arrived
    sem_t mutex;           // Protects the count variable
    sem_t barrier;         // Used to block threads at the barrier
} barrier_t;


// the single barrier we are using for this program
barrier_t b;

void barrier_init(barrier_t *b, int num_threads) {
    b->num_threads = num_threads;
    b->count = 0;
    sem_init(&b->mutex, 0, 1);      // Binary semaphore (mutex) initialized to 1
    sem_init(&b->barrier, 0, 0);    // Barrier semaphore initialized to 0
}

void barrier(barrier_t *b) {
    sem_wait(&b->mutex);            // Lock to safely update count
    b->count++;
    if (b->count == b->num_threads) {
        // Last thread to arrive: release all waiting threads
        for (int i = 0; i < b->num_threads; i++) {
            sem_post(&b->barrier);
        }
    }
    sem_post(&b->mutex);            // Unlock
    
    sem_wait(&b->barrier);          // Wait at the barrier until released
}

//
// XXX: don't change below here (just run it!)
//
typedef struct __tinfo_t {
    int thread_id;
} tinfo_t;

void *child(void *arg) {
    tinfo_t *t = (tinfo_t *) arg;
    printf("child %d: before\n", t->thread_id);
    barrier(&b);
    printf("child %d: after\n", t->thread_id);
    return NULL;
}


// run with a single argument indicating the number of 
// threads you wish to create (1 or more)
int main(int argc, char *argv[]) {
    assert(argc == 2);
    int num_threads = atoi(argv[1]);
    assert(num_threads > 0);

    pthread_t p[num_threads];
    tinfo_t t[num_threads];

    printf("parent: begin\n");
    barrier_init(&b, num_threads);
    
    int i;
    for (i = 0; i < num_threads; i++) {
	t[i].thread_id = i;
	Pthread_create(&p[i], NULL, child, &t[i]);
    }

    for (i = 0; i < num_threads; i++) 
	Pthread_join(p[i], NULL);

    printf("parent: end\n");
    return 0;
}
