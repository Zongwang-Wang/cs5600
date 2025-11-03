#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct node {
    int key;
    struct node *next;
    pthread_mutex_t lock;
} node_t;

typedef struct {
    node_t *head;
    pthread_mutex_t lock;
} list_t;

// === Standard List (one global lock) ===

void list_init(list_t *list) {
    list->head = NULL;
    pthread_mutex_init(&list->lock, NULL);
}

int list_lookup(list_t *list, int key) {
    pthread_mutex_lock(&list->lock);
    node_t *curr = list->head;
    while (curr) {
        if (curr->key == key) {
            pthread_mutex_unlock(&list->lock);
            return 1;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&list->lock);
    return 0;
}

// === Hand-Over-Hand List (per-node locks) ===

void hoh_init(list_t *list) {
    list->head = malloc(sizeof(node_t));
    list->head->key = -1;
    list->head->next = NULL;
    pthread_mutex_init(&list->head->lock, NULL);
}

int hoh_lookup(list_t *list, int key) {
    pthread_mutex_lock(&list->head->lock);
    node_t *curr = list->head;
    
    while (curr->next) {
        pthread_mutex_lock(&curr->next->lock);
        pthread_mutex_unlock(&curr->lock);
        curr = curr->next;
        
        if (curr->key == key) {
            pthread_mutex_unlock(&curr->lock);
            return 1;
        }
    }
    
    pthread_mutex_unlock(&curr->lock);
    return 0;
}

// === Benchmark ===

typedef struct {
    list_t *list;
    int *keys;
    int num_ops;
    int use_hoh;
} arg_t;

void *worker(void *arg) {
    arg_t *a = (arg_t *)arg;
    for (int i = 0; i < a->num_ops; i++) {
        if (a->use_hoh) {
            hoh_lookup(a->list, a->keys[i]);
        } else {
            list_lookup(a->list, a->keys[i]);
        }
    }
    return NULL;
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void run_test(int use_hoh, int num_threads, int list_size, int num_ops) {
    list_t list;
    
    // Initialize and populate
    if (use_hoh) {
        hoh_init(&list);
    } else {
        list_init(&list);
    }
    
    for (int i = list_size - 1; i >= 0; i--) {
        node_t *n = malloc(sizeof(node_t));
        n->key = i;
        pthread_mutex_init(&n->lock, NULL);
        n->next = list.head;
        list.head = n;
    }
    
    // Setup threads
    pthread_t threads[num_threads];
    arg_t args[num_threads];
    
    for (int i = 0; i < num_threads; i++) {
        args[i].list = &list;
        args[i].num_ops = num_ops;
        args[i].use_hoh = use_hoh;
        args[i].keys = malloc(num_ops * sizeof(int));
        for (int j = 0; j < num_ops; j++) {
            args[i].keys[j] = rand() % list_size;
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
    
    printf("%s: %.4f sec\n", use_hoh ? "Hand-Over-Hand" : "Standard     ", time);
    
    // Cleanup
    for (int i = 0; i < num_threads; i++) {
        free(args[i].keys);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <threads> <list_size> <lookups>\n", argv[0]);
        return 1;
    }
    
    int threads = atoi(argv[1]);
    int size = atoi(argv[2]);
    int ops = atoi(argv[3]);
    
    printf("Threads: %d, List: %d, Lookups: %d\n", threads, size, ops);
    run_test(0, threads, size, ops);
    run_test(1, threads, size, ops);
    
    return 0;
}