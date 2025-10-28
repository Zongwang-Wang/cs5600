// test_spork.c - Enhanced test program with real behavior
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

// Include spork API
extern void spork_set_next_fork_pattern(int pattern);
extern void spork_set_next_exec_params(const char *path, char *const argv[], char *const envp[]);

#define PATTERN_FORK_EXEC 1
#define PATTERN_WORKER 2
#define PATTERN_SNAPSHOT 3

// Helper function to measure time
double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

void separator() {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

// ============================================================================
// TEST 1: P1 Pattern - Fork + Exec with echo
// ============================================================================
void test_p1_simple_exec() {
    printf("TEST 1: P1 Pattern - Simple Fork + Exec\n");
    printf("Expected: Facade should use posix_spawn\n");
    separator();
    
    double start = get_time_ms();
    
    // HINT: Tell Spork this is P1 pattern
    spork_set_next_fork_pattern(PATTERN_FORK_EXEC);
    
    // HINT: Tell Spork what will be exec'd
    char *argv[] = {"echo", "Hello from P1 pattern!", NULL};
    spork_set_next_exec_params("/bin/echo", argv, NULL);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child: This code won't actually run because posix_spawn
        // directly executes the program
        printf("[Child] This message should NOT appear (posix_spawn)\n");
        exit(1);
    } else {
        // Parent
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("\n[Parent] Child exited with status: %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
    
    separator();
}

// ============================================================================
// TEST 2: P1 Pattern - Fork + Exec with ls
// ============================================================================
void test_p1_exec_with_args() {
    printf("TEST 2: P1 Pattern - Fork + Exec with Arguments\n");
    printf("Expected: Facade should use posix_spawn to run 'ls -lh'\n");
    separator();
    
    double start = get_time_ms();
    
    // HINT: P1 pattern with ls command
    spork_set_next_fork_pattern(PATTERN_FORK_EXEC);
    char *argv[] = {"ls", "-lh", NULL};
    spork_set_next_exec_params("/bin/ls", argv, NULL);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // This won't run with posix_spawn
        printf("[Child] Should not see this\n");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("\n[Parent] Child exited with status: %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
    
    separator();
}

// ============================================================================
// TEST 3: P2 Pattern - Worker Process (NO EXEC)
// ============================================================================
void test_p2_worker() {
    printf("TEST 3: P2 Pattern - Worker Process (NO EXEC)\n");
    printf("Expected: uFork should use REAL fork\n");
    separator();
    
    double start = get_time_ms();
    
    // HINT: This is P2 - worker pattern, NO exec
    spork_set_next_fork_pattern(PATTERN_WORKER);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child: This WILL run because we're using real fork
        printf("[Child Worker] Starting work in PID: %d\n", getpid());
        
        // Simulate some computation
        int sum = 0;
        for (int i = 0; i < 1000000; i++) {
            sum += i % 100;
        }
        
        printf("[Child Worker] Computation done. Result: %d\n", sum);
        printf("[Child Worker] Exiting...\n");
        exit(0);
    } else {
        // Parent
        printf("[Parent] Waiting for worker...\n");
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("[Parent] Worker exited with status: %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
    
    separator();
}

// ============================================================================
// TEST 4: P2 Pattern - Multiple Workers
// ============================================================================
void test_p2_multiple_workers() {
    printf("TEST 4: P2 Pattern - Multiple Worker Processes\n");
    printf("Expected: Each fork should use REAL fork\n");
    separator();
    
    const int NUM_WORKERS = 3;
    pid_t pids[NUM_WORKERS];
    
    double start = get_time_ms();
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        // HINT: Worker pattern for each fork
        spork_set_next_fork_pattern(PATTERN_WORKER);
        
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            return;
        } else if (pid == 0) {
            // Child worker
            printf("[Worker %d] Started in PID: %d\n", i, getpid());
            
            // Each worker does different amount of work
            int result = 0;
            for (int j = 0; j < 500000 * (i + 1); j++) {
                result += j % 100;
            }
            
            printf("[Worker %d] Finished. Result: %d\n", i, result);
            exit(0);
        } else {
            // Parent
            pids[i] = pid;
        }
    }
    
    // Wait for all workers
    printf("[Parent] Waiting for all %d workers...\n", NUM_WORKERS);
    for (int i = 0; i < NUM_WORKERS; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        printf("[Parent] Worker %d (PID: %d) exited with status: %d\n", 
               i, pids[i], WEXITSTATUS(status));
    }
    
    double end = get_time_ms();
    printf("[Parent] All workers done. Total time: %.2f ms\n", end - start);
    
    separator();
}

// ============================================================================
// TEST 5: P3 Pattern - Snapshot
// ============================================================================
void test_p3_snapshot() {
    printf("TEST 5: P3 Pattern - Snapshot (Copy-on-Write)\n");
    printf("Expected: uFork should use REAL fork with COW\n");
    separator();
    
    // Simulate important data
    int important_data = 12345;
    printf("[Parent] Data before fork: %d\n", important_data);
    
    double start = get_time_ms();
    
    // HINT: Snapshot pattern
    spork_set_next_fork_pattern(PATTERN_SNAPSHOT);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child: Has snapshot of parent's memory
        printf("[Child Snapshot] Got snapshot of data: %d\n", important_data);
        printf("[Child Snapshot] Simulating save to disk...\n");
        sleep(1);
        printf("[Child Snapshot] Snapshot saved successfully!\n");
        exit(0);
    } else {
        // Parent: Continues with modified data
        printf("[Parent] Child PID: %d is taking snapshot...\n", pid);
        printf("[Parent] Continuing work while snapshot in progress...\n");
        
        // Modify data while child is snapshotting
        important_data = 99999;
        printf("[Parent] Modified data to: %d (child has old value)\n", important_data);
        
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("[Parent] Snapshot completed with status: %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
    
    separator();
}

// ============================================================================
// TEST 6: Comparison - P1 vs P2 Performance
// ============================================================================
void test_performance_comparison() {
    printf("TEST 6: Performance Comparison - P1 (Facade) vs P2 (uFork)\n");
    separator();
    
    const int ITERATIONS = 10;
    double p1_total = 0.0;
    double p2_total = 0.0;
    
    // Test P1 pattern
    printf("Running P1 pattern %d times...\n", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        double start = get_time_ms();
        
        spork_set_next_fork_pattern(PATTERN_FORK_EXEC);
        char *argv[] = {"echo", "test", NULL};
        spork_set_next_exec_params("/bin/echo", argv, NULL);
        
        pid_t pid = fork();
        if (pid == 0) exit(0);
        if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            p1_total += get_time_ms() - start;
        }
    }
    
    // Test P2 pattern
    printf("Running P2 pattern %d times...\n", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        double start = get_time_ms();
        
        spork_set_next_fork_pattern(PATTERN_WORKER);
        
        pid_t pid = fork();
        if (pid == 0) exit(0);
        if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            p2_total += get_time_ms() - start;
        }
    }
    
    printf("\n=== Performance Results ===\n");
    printf("P1 (Facade/posix_spawn): %.2f ms average\n", p1_total / ITERATIONS);
    printf("P2 (uFork/real fork):    %.2f ms average\n", p2_total / ITERATIONS);
    printf("Speedup:                 %.2fx\n", p2_total / p1_total);
    
    separator();
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                 SPORK COMPREHENSIVE TEST SUITE               ║\n");
    printf("║                  Testing All Fork Patterns                   ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    test_p1_simple_exec();
    test_p1_exec_with_args();
    test_p2_worker();
    test_p2_multiple_workers();
    test_p3_snapshot();
    test_performance_comparison();
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                    ALL TESTS COMPLETED                        ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}