#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define BUCKETS 101

typedef struct node {
    int key;
    int value;
    struct node *next;
} node_t;

// Global lock hash table
typedef struct {
    node_t *table[BUCKETS];
    pthread_mutex_t lock;
} hash_global_t;

// Per-bucket lock hash table
typedef struct {
    node_t *table[BUCKETS];
    pthread_mutex_t locks[BUCKETS];
} hash_bucket_t;

int hash(int key) {
    return key % BUCKETS;
}

// === Global Lock Implementation ===

void hash_global_init(hash_global_t *h) {
    for (int i = 0; i < BUCKETS; i++) {
        h->table[i] = NULL;
    }
    pthread_mutex_init(&h->lock, NULL);
}

void hash_global_insert(hash_global_t *h, int key, int value) {
    int bucket = hash(key);
    node_t *n = malloc(sizeof(node_t));
    n->key = key;
    n->value = value;
    
    pthread_mutex_lock(&h->lock);
    n->next = h->table[bucket];
    h->table[bucket] = n;
    pthread_mutex_unlock(&h->lock);
}

int hash_global_lookup(hash_global_t *h, int key) {
    int bucket = hash(key);
    
    pthread_mutex_lock(&h->lock);
    node_t *curr = h->table[bucket];
    while (curr) {
        if (curr->key == key) {
            int val = curr->value;
            pthread_mutex_unlock(&h->lock);
            return val;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&h->lock);
    return -1;
}

// === Per-Bucket Lock Implementation ===

void hash_bucket_init(hash_bucket_t *h) {
    for (int i = 0; i < BUCKETS; i++) {
        h->table[i] = NULL;
        pthread_mutex_init(&h->locks[i], NULL);
    }
}

void hash_bucket_insert(hash_bucket_t *h, int key, int value) {
    int bucket = hash(key);
    node_t *n = malloc(sizeof(node_t));
    n->key = key;
    n->value = value;
    
    pthread_mutex_lock(&h->locks[bucket]);
    n->next = h->table[bucket];
    h->table[bucket] = n;
    pthread_mutex_unlock(&h->locks[bucket]);
}

int hash_bucket_lookup(hash_bucket_t *h, int key) {
    int bucket = hash(key);
    
    pthread_mutex_lock(&h->locks[bucket]);
    node_t *curr = h->table[bucket];
    while (curr) {
        if (curr->key == key) {
            int val = curr->value;
            pthread_mutex_unlock(&h->locks[bucket]);
            return val;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&h->locks[bucket]);
    return -1;
}

// === Benchmark ===

typedef struct {
    void *hash;
    int *keys;
    int num_ops;
    int use_bucket;
} arg_t;

void *worker(void *arg) {
    arg_t *a = (arg_t *)arg;
    
    for (int i = 0; i < a->num_ops; i++) {
        if (a->use_bucket) {
            hash_bucket_lookup((hash_bucket_t *)a->hash, a->keys[i]);
        } else {
            hash_global_lookup((hash_global_t *)a->hash, a->keys[i]);
        }
    }
    return NULL;
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void run_test(int use_bucket, int num_threads, int num_items, int num_ops) {
    hash_global_t hg;
    hash_bucket_t hb;
    
    // Initialize
    if (use_bucket) {
        hash_bucket_init(&hb);
        for (int i = 0; i < num_items; i++) {
            hash_bucket_insert(&hb, i, i * 10);
        }
    } else {
        hash_global_init(&hg);
        for (int i = 0; i < num_items; i++) {
            hash_global_insert(&hg, i, i * 10);
        }
    }
    
    // Setup threads
    pthread_t threads[num_threads];
    arg_t args[num_threads];
    
    for (int i = 0; i < num_threads; i++) {
        args[i].hash = use_bucket ? (void *)&hb : (void *)&hg;
        args[i].num_ops = num_ops;
        args[i].use_bucket = use_bucket;
        args[i].keys = malloc(num_ops * sizeof(int));
        for (int j = 0; j < num_ops; j++) {
            args[i].keys[j] = rand() % num_items;
        }
    }
    
    // Run
    double start = get_time();
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    double time = get_time() - start;
    
    printf("%s: %.4f sec\n", 
           use_bucket ? "Per-Bucket Lock" : "Global Lock    ", time);
    
    // Cleanup
    for (int i = 0; i < num_threads; i++) {
        free(args[i].keys);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <threads> <items> <lookups>\n", argv[0]);
        return 1;
    }
    
    int threads = atoi(argv[1]);
    int items = atoi(argv[2]);
    int ops = atoi(argv[3]);
    
    printf("Threads: %d, Items: %d, Lookups: %d, Buckets: %d\n", 
           threads, items, ops, BUCKETS);
    run_test(0, threads, items, ops);
    run_test(1, threads, items, ops);
    
    return 0;
}