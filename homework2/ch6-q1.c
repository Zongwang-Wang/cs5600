#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

int main() {
    struct timeval start, end;
    int iterations = 1000000;
    
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < iterations; i++) {
        read(0, NULL, 0);  // Simple system call
    }
    
    gettimeofday(&end, NULL);
    
    long elapsed = (end.tv_sec - start.tv_sec) * 1000000 + 
                   (end.tv_usec - start.tv_usec);
    
    printf("System call cost: %.3f microseconds\n", 
           (double)elapsed / iterations);
    
    return 0;
}
