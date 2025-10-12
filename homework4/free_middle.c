#include <stdio.h>
#include <stdlib.h>

int main() {
    // Allocate array of 100 integers
    int *data = (int *)malloc(100 * sizeof(int));
    
    if (data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize some values
    data[0] = 42;
    data[50] = 100;
    
    printf("Allocated memory at: %p\n", (void *)data);
    printf("Middle of array at: %p\n", (void *)&data[50]);
    
    // Try to free a pointer in the middle of the array - THIS IS A BUG!
    free(&data[50]);
    
    printf("Program completed\n");
    
    return 0;
}
