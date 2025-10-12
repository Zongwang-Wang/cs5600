#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *data;      // pointer to array of elements
    size_t size;    // current number of elements
    size_t capacity;// total allocated space
} Vector;

// Initialize vector
void vector_init(Vector *v) {
    v->size = 0;
    v->capacity = 2;  // start small
    v->data = malloc(v->capacity * sizeof(int));
    if (!v->data) {
        perror("malloc");
        exit(1);
    }
}

// Add element, grow array if needed
void vector_push(Vector *v, int value) {
    if (v->size == v->capacity) {
        v->capacity *= 2;
        int *new_data = realloc(v->data, v->capacity * sizeof(int));
        if (!new_data) {
            perror("realloc");
            free(v->data);
            exit(1);
        }
        v->data = new_data;
    }
    v->data[v->size++] = value;
}

// Free memory
void vector_free(Vector *v) {
    free(v->data);
    v->data = NULL;
    v->size = v->capacity = 0;
}

int main(void) {
    Vector v;
    vector_init(&v);

    for (int i = 0; i < 10; i++) {
        vector_push(&v, i * 10);
        printf("Added %d (size=%zu, capacity=%zu)\n", i * 10, v.size, v.capacity);
    }

    printf("\nVector contents:\n");
    for (size_t i = 0; i < v.size; i++) {
        printf("%d ", v.data[i]);
    }
    printf("\n");

    vector_free(&v);
    return 0;
}
