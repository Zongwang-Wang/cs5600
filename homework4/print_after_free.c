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
    
    // Free the memory
    free(data);
    
    // Try to print a value after freeing - THIS IS A BUG!
    printf("Value at data[50]: %d\n", data[50]);
    
    return 0;
}
