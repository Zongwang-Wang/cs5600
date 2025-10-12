#include <stdio.h>
#include <stdlib.h>

int main() {
    // Allocate array of 100 integers
    int *data = (int *)malloc(100 * sizeof(int));
    
    if (data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Set data[100] to zero - THIS IS A BUG!
    data[100] = 0;
    
    printf("Program completed successfully\n");
    
    free(data);
    return 0;
}

