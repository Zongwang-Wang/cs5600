#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

int main() {
    int pipe1[2], pipe2[2];
    struct timeval start, end;
    int iterations = 10000;
    char c = 'x';
    
    pipe(pipe1);
    pipe(pipe2);
    
    // First: Measure just system call overhead (no context switch)
    printf("Measuring system call overhead...\n");
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < iterations; i++) {
        write(pipe1[1], &c, 1);
        read(pipe1[0], &c, 1);  // Same process, no context switch
    }
    
    gettimeofday(&end, NULL);
    long syscall_time = (end.tv_sec - start.tv_sec) * 1000000 + 
                        (end.tv_usec - start.tv_usec);
    
    // Second: Measure with context switches
    printf("Measuring with context switches...\n");
    
    if (fork() == 0) {
        // Child
        for (int i = 0; i < iterations; i++) {
            read(pipe1[0], &c, 1);
            write(pipe2[1], &c, 1);
        }
        return 0;
    }
    
    // Parent
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < iterations; i++) {
        write(pipe1[1], &c, 1);
        read(pipe2[0], &c, 1);
    }
    
    gettimeofday(&end, NULL);
    
    long total_time = (end.tv_sec - start.tv_sec) * 1000000 + 
                      (end.tv_usec - start.tv_usec);
    
    // Calculate results
    printf("\n--- Results ---\n");
    printf("System call overhead: %.3f microseconds\n", 
           (double)syscall_time / iterations);
    printf("Total with context switches: %.3f microseconds\n", 
           (double)total_time / iterations);
    printf("Pure context switch cost: %.3f microseconds\n", 
           (double)(total_time - syscall_time) / (iterations * 2));
    
    return 0;
}
