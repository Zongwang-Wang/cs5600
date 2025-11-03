#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    long value;
    pthread_mutex_t lock;
} counter_t;

typedef struct {
    counter_t *counter;
    int num_increments;
} thread_arg_t;

void counter_init(counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void counter_increment(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

long counter_get(counter_t *c) {
    return c->value;
}

void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    for (int i = 0; i < a->num_increments; i++) {
        counter_increment(a->counter);
    }
    return NULL;
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <num_increments>\n", argv[0]);
        return 1;
    }
    
    int num_threads = atoi(argv[1]);
    int num_increments = atoi(argv[2]);
    
    counter_t counter;
    counter_init(&counter);
    
    pthread_t threads[num_threads];
    thread_arg_t args[num_threads];
    
    for (int i = 0; i < num_threads; i++) {
        args[i].counter = &counter;
        args[i].num_increments = num_increments;
    }
    
    double start = get_time();
    
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double end = get_time();
    
    printf("Threads: %d, Time: %.4f sec, Counter: %ld\n", 
           num_threads, end - start, counter_get(&counter));
    
    return 0;
}