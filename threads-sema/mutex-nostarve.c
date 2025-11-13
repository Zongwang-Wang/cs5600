#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common_threads.h"

//
// Here, you have to write (almost) ALL the code. Oh no!
// How can you show that a thread does not starve
// when attempting to acquire this mutex you build?
//

typedef struct __ns_mutex_t {
	sem_t mutex;  // Protects the waiting counter
	sem_t room;   // The actual lock (1 = available, 0 = taken)
	sem_t queue;  // FIFO queue for waiting threads
	int waiting;  // Number of threads waiting
} ns_mutex_t;

int counter = 0;  // Shared counter to test mutex
ns_mutex_t m;

void ns_mutex_init(ns_mutex_t *m) {
	m->waiting = 0;
	sem_init(&m->mutex, 0, 1);  // Binary semaphore for protecting counter
	sem_init(&m->room, 0, 1);   // The actual lock (starts available)
	sem_init(&m->queue, 0, 0);  // Queue starts empty
}

void ns_mutex_acquire(ns_mutex_t *m) {
	sem_wait(&m->mutex);  // Protect the waiting counter

	if (m->waiting > 0 || sem_trywait(&m->room) != 0) {
		// Either someone is waiting OR the room is taken
		// Join the queue to ensure FIFO ordering
		m->waiting++;
		sem_post(&m->mutex);  // Release counter protection
		sem_wait(&m->queue);  // Wait in FIFO queue
				      // When woken up, we have the lock
	} else {
		// Got the room immediately (sem_trywait succeeded)
		sem_post(&m->mutex);  // Release counter protection
	}
}

void ns_mutex_release(ns_mutex_t *m) {
	sem_wait(&m->mutex);  // Protect the waiting counter

	if (m->waiting > 0) {
		// Wake up next thread in queue
		m->waiting--;
		sem_post(&m->queue);  // Pass lock directly to next waiter
	} else {
		// No one waiting, release the room
		sem_post(&m->room);  // Make room available
	}

	sem_post(&m->mutex);  // Release counter protection
}

void *worker(void *arg) {
	int thread_id = *(int *)arg;
	int i;

	for (i = 0; i < 5; i++) {
		printf("Thread %d: attempting to acquire mutex\n", thread_id);
		ns_mutex_acquire(&m);

		// Critical section
		printf("Thread %d: ACQUIRED mutex (counter = %d)\n", thread_id,
		       counter);
		counter++;
		sleep(1);  // Simulate work to make starvation visible
		printf("Thread %d: RELEASING mutex (counter = %d)\n", thread_id,
		       counter);

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

	printf("Final counter value: %d (expected: %d)\n", counter,
	       num_threads * 5);
	printf("parent: end\n");
	return 0;
}
