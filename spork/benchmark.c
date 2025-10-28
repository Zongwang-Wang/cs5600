// benchmark.c - Traditional fork() benchmark (NO Spork)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

void separator() {
    printf("═══════════════════════════════════════════════════════════════\n");
}

// ============================================================================
// BENCHMARK 1: Fork + Exec Pattern (P1)
// ============================================================================
void benchmark_fork_exec(int iterations) {
    printf("\nBENCHMARK 1: Fork + Exec Pattern\n");
    printf("Iterations: %d\n", iterations);
    separator();
    
    double total_time = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        double start = get_time_ms();
        
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child: exec echo
            char *argv[] = {"echo", "benchmark", NULL};
            execv("/bin/echo", argv);
            perror("execv");
            exit(1);
        } else {
            // Parent: wait
            int status;
            waitpid(pid, &status, 0);
            total_time += get_time_ms() - start;
        }
    }
    
    printf("Total time: %.2f ms\n", total_time);
    printf("Average per fork+exec: %.2f ms\n", total_time / iterations);
    separator();
}

// ============================================================================
// BENCHMARK 2: Fork + Exec with Arguments
// ============================================================================
void benchmark_fork_exec_args(int iterations) {
    printf("\nBENCHMARK 2: Fork + Exec with Arguments\n");
    printf("Iterations: %d\n", iterations);
    separator();
    
    double total_time = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        double start = get_time_ms();
        
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child: exec ls with args
            char *argv[] = {"ls", "-lh", "/tmp", NULL};
            execv("/bin/ls", argv);
            perror("execv");
            exit(1);
        } else {
            // Parent: wait
            int status;
            waitpid(pid, &status, 0);
            total_time += get_time_ms() - start;
        }
    }
    
    printf("Total time: %.2f ms\n", total_time);
    printf("Average per fork+exec: %.2f ms\n", total_time / iterations);
    separator();
}

// ============================================================================
// BENCHMARK 3: Worker Process (P2)
// ============================================================================
void benchmark_worker(int iterations) {
    printf("\nBENCHMARK 3: Worker Process (No Exec)\n");
    printf("Iterations: %d\n", iterations);
    separator();
    
    double total_time = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        double start = get_time_ms();
        
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child: Do some work
            int sum = 0;
            for (int j = 0; j < 1000000; j++) {
                sum += j % 100;
            }
            exit(0);
        } else {
            // Parent: wait
            int status;
            waitpid(pid, &status, 0);
            total_time += get_time_ms() - start;
        }
    }
    
    printf("Total time: %.2f ms\n", total_time);
    printf("Average per fork: %.2f ms\n", total_time / iterations);
    separator();
}

// ============================================================================
// BENCHMARK 4: Multiple Workers
// ============================================================================
void benchmark_multiple_workers(int num_workers) {
    printf("\nBENCHMARK 4: Multiple Workers\n");
    printf("Number of workers: %d\n", num_workers);
    separator();
    
    double start = get_time_ms();
    pid_t *pids = malloc(num_workers * sizeof(pid_t));
    
    // Fork all workers
    for (int i = 0; i < num_workers; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child worker
            int result = 0;
            for (int j = 0; j < 500000 * (i + 1); j++) {
                result += j % 100;
            }
            exit(0);
        } else {
            pids[i] = pid;
        }
    }
    
    // Wait for all workers
    for (int i = 0; i < num_workers; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    
    double total_time = get_time_ms() - start;
    free(pids);
    
    printf("Total time: %.2f ms\n", total_time);
    printf("Average per worker: %.2f ms\n", total_time / num_workers);
    separator();
}

// ============================================================================
// BENCHMARK 5: Memory Snapshot Pattern
// ============================================================================
void benchmark_snapshot(int iterations) {
    printf("\nBENCHMARK 5: Memory Snapshot Pattern\n");
    printf("Iterations: %d\n", iterations);
    separator();
    
    double total_time = 0.0;
    
    // Allocate some memory to snapshot
    size_t size = 10 * 1024 * 1024; // 10 MB
    char *data = malloc(size);
    memset(data, 0xAB, size);
    
    for (int i = 0; i < iterations; i++) {
        double start = get_time_ms();
        
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child: Read the snapshot
            volatile char checksum = 0;
            for (size_t j = 0; j < size; j += 4096) {
                checksum ^= data[j];
            }
            exit(0);
        } else {
            // Parent: wait
            int status;
            waitpid(pid, &status, 0);
            total_time += get_time_ms() - start;
        }
    }
    
    free(data);
    
    printf("Total time: %.2f ms\n", total_time);
    printf("Average per snapshot: %.2f ms\n", total_time / iterations);
    separator();
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char *argv[]) {
    int iterations = 10;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║         TRADITIONAL FORK() BENCHMARK (NO SPORK)              ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
    benchmark_fork_exec(iterations);
    benchmark_fork_exec_args(iterations);
    benchmark_worker(iterations);
    benchmark_multiple_workers(5);
    benchmark_snapshot(5);
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                  BENCHMARK COMPLETED                          ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}