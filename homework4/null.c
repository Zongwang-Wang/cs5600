#include <stdio.h>

int main() {
    int *ptr = NULL;
    
    printf("About to dereference a NULL pointer...\n");
    
    // This will cause a segmentation fault
    printf("Value: %d\n", *ptr);
    
    return 0;
}
