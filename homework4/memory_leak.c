#include <stdio.h>
#include <stdlib.h>

int main() {
    int *array;
    int size = 100;
    
    printf("Allocating memory for %d integers...\n", size);
    
    // Allocate memory
    array = (int *)malloc(size * sizeof(int));
    
    if (array == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }
    
    // Use the allocated memory
    for (int i = 0; i < size; i++) {
        array[i] = i * 2;
    }
    
    printf("First element: %d\n", array[0]);
    printf("Last element: %d\n", array[size - 1]);
    
    // BUG: Forgot to free(array)!
    printf("Exiting program without freeing memory...\n");
    
    return 0;
}
