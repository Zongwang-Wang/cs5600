#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t {
    sem_t lock;          // Protects readers counter
    sem_t writelock;     // Ensures exclusive writer access
    sem_t turnstile;     // Ensures fairness (FIFO ordering)
    int readers;         // Count of active readers
} rwlock_t;


void rwlock_init(rwlock_t *rw) {
    rw->readers = 0;
    sem_init(&rw->lock, 0, 1);
    sem_init(&rw->writelock, 0, 1);
    sem_init(&rw->turnstile, 0, 1);  // Turnstile for fairness
}

void rwlock_acquire_readlock(rwlock_t *rw) {
    sem_wait(&rw->turnstile);     // Wait in line (fairness)
    sem_post(&rw->turnstile);     // Immediately release (readers don't block each other)
    
    sem_wait(&rw->lock);
    rw->readers++;
    if (rw->readers == 1) {
        // First reader locks out writers
        sem_wait(&rw->writelock);
    }
    sem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw) {
    sem_wait(&rw->lock);
    rw->readers--;
    if (rw->readers == 0) {
        // Last reader unlocks for writers
        sem_post(&rw->writelock);
    }
    sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw) {
    sem_wait(&rw->turnstile);     // Wait in line (fairness)
    sem_wait(&rw->writelock);     // Acquire exclusive access
    // Note: turnstile is NOT released yet - blocks all new readers/writers
}

void rwlock_release_writelock(rwlock_t *rw) {
    sem_post(&rw->turnstile);     // Release turnstile for next in line
    sem_post(&rw->writelock);     // Release exclusive access
}

//
// Don't change the code below (just use it!)
// 

int loops;
int value = 0;

rwlock_t lock;

void *reader(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
	rwlock_acquire_readlock(&lock);
	printf("read %d\n", value);
	rwlock_release_readlock(&lock);
    }
    return NULL;
}

void *writer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
	rwlock_acquire_writelock(&lock);
	value++;
	printf("write %d\n", value);
	rwlock_release_writelock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);
    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    loops = atoi(argv[3]);

    pthread_t pr[num_readers], pw[num_writers];

    rwlock_init(&lock);

    printf("begin\n");

    int i;
    for (i = 0; i < num_readers; i++)
	Pthread_create(&pr[i], NULL, reader, NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_create(&pw[i], NULL, writer, NULL);

    for (i = 0; i < num_readers; i++)
	Pthread_join(pr[i], NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_join(pw[i], NULL);

    printf("end: value %d\n", value);

    return 0;
}
