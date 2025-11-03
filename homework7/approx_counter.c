#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_THREADS 64

typedef struct {
    pthread_mutex_t lock;
    long value;
} counter_t;

typedef struct {
    pthread_mutex_t locks[MAX_THREADS];
    long local[MAX_THREADS];
    counter_t global;
    int threshold;
    int num_cpus;
} approx_counter_t;

typedef struct {
    approx_counter_t *counter;
    int thread_id;
    int num_increments;
} thread_arg_t;

void init_counter(counter_t *c) {
    pthread_mutex_init(&c->lock, NULL);
    c->value = 0;
}

void init_approx_counter(approx_counter_t *ac, int threshold, int num_cpus) {
    init_counter(&ac->global);
    ac->threshold = threshold;
    ac->num_cpus = num_cpus;
    for (int i = 0; i < num_cpus; i++) {
        pthread_mutex_init(&ac->locks[i], NULL);
        ac->local[i] = 0;
    }
}

void approx_increment(approx_counter_t *ac, int thread_id) {
    pthread_mutex_lock(&ac->locks[thread_id]);
    ac->local[thread_id]++;
    
    if (ac->local[thread_id] >= ac->threshold) {
        pthread_mutex_lock(&ac->global.lock);
        ac->global.value += ac->local[thread_id];
        pthread_mutex_unlock(&ac->global.lock);
        ac->local[thread_id] = 0;
    }
    
    pthread_mutex_unlock(&ac->locks[thread_id]);
}

long approx_get(approx_counter_t *ac) {
    pthread_mutex_lock(&ac->global.lock);
    long total = ac->global.value;
    pthread_mutex_unlock(&ac->global.lock);
    
    for (int i = 0; i < ac->num_cpus; i++) {
        pthread_mutex_lock(&ac->locks[i]);
        total += ac->local[i];
        pthread_mutex_unlock(&ac->locks[i]);
    }
    
    return total;
}

void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    for (int i = 0; i < a->num_increments; i++) {
        approx_increment(a->counter, a->thread_id);
    }
    return NULL;
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <num_threads> <num_increments> <threshold>\n", argv[0]);
        return 1;
    }
    
    int num_threads = atoi(argv[1]);
    int num_increments = atoi(argv[2]);
    int threshold = atoi(argv[3]);
    
    approx_counter_t counter;
    init_approx_counter(&counter, threshold, num_threads);
    
    pthread_t threads[num_threads];
    thread_arg_t args[num_threads];
    
    for (int i = 0; i < num_threads; i++) {
        args[i].counter = &counter;
        args[i].thread_id = i;
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
    long final = approx_get(&counter);
    
    printf("Threads: %d, Threshold: %d, Time: %.4f sec, Counter: %ld\n", 
           num_threads, threshold, end - start, final);
    
    return 0;
}