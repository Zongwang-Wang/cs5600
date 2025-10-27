// test_spork.c - Enhanced test program
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

// Helper function to measure time
double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

void test_simple_fork_exit() {
    printf("\n=== Test 1: Simple Fork + Exit (Simulated Fork-Exec) ===\n");
    printf("Pattern: P1 - Fork followed by exit (simulates fork-exec)\n");
    
    double start = get_time_ms();
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child process
        printf("[Child] In child process (PID: %d)\n", getpid());
        printf("[Child] Exiting immediately...\n");
        exit(0);
    } else {
        // Parent process
        printf("[Parent] Created child with PID: %d\n", pid);
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("[Parent] Child exited with status %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
}

void test_fork_exec() {
    printf("\n=== Test 2: Fork + Exec (Real P1 Pattern) ===\n");
    printf("Pattern: P1 - Fork followed by exec\n");
    
    double start = get_time_ms();
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child process - execute a real program
        printf("[Child] About to exec /bin/echo...\n");
        execl("/bin/echo", "echo", "Child executed via exec!", NULL);
        // If exec fails
        perror("execl");
        exit(1);
    } else {
        // Parent process
        printf("[Parent] Created child with PID: %d\n", pid);
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("[Parent] Child exited with status %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
}

void test_fork_exec_with_args() {
    printf("\n=== Test 3: Fork + Exec with Arguments ===\n");
    printf("Pattern: P1 - Fork followed by exec with file listing\n");
    
    double start = get_time_ms();
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child process - execute ls command
        printf("[Child] Executing ls -lh (current directory)...\n");
        execl("/bin/ls", "ls", "-lh", NULL);
        // If exec fails
        perror("execl");
        exit(1);
    } else {
        // Parent process
        printf("[Parent] Created child with PID: %d\n", pid);
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("[Parent] Child exited with status %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
}

void test_worker_process() {
    printf("\n=== Test 4: Worker Process (P2 Pattern) ===\n");
    printf("Pattern: P2 - Fork to create worker (NO exec)\n");
    printf("Note: This will fall back to uFork since no exec is detected\n");
    
    double start = get_time_ms();
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child process - worker that doesn't exec
        printf("[Child Worker] Starting work (PID: %d)...\n", getpid());
        
        // Simulate some work
        int sum = 0;
        for (int i = 0; i < 1000000; i++) {
            sum += i;
        }
        
        printf("[Child Worker] Completed work. Result: %d\n", sum);
        printf("[Child Worker] Exiting...\n");
        exit(0);
    } else {
        // Parent process
        printf("[Parent] Created worker child with PID: %d\n", pid);
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("[Parent] Worker exited with status %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
}

void test_multiple_workers() {
    printf("\n=== Test 5: Multiple Worker Processes (P2 Pattern) ===\n");
    printf("Pattern: P2 - Create multiple workers for parallel work\n");
    
    const int NUM_WORKERS = 3;
    pid_t pids[NUM_WORKERS];
    
    double start = get_time_ms();
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            return;
        } else if (pid == 0) {
            // Child worker process
            printf("[Worker %d] Started (PID: %d)\n", i, getpid());
            
            // Each worker does some computation
            int result = 0;
            for (int j = 0; j < 500000 * (i + 1); j++) {
                result += j % 100;
            }
            
            printf("[Worker %d] Finished with result: %d\n", i, result);
            exit(0);
        } else {
            // Parent process
            pids[i] = pid;
            printf("[Parent] Spawned worker %d with PID: %d\n", i, pid);
        }
    }
    
    // Parent waits for all workers
    printf("[Parent] Waiting for all workers to complete...\n");
    for (int i = 0; i < NUM_WORKERS; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        printf("[Parent] Worker %d (PID: %d) completed with status %d\n", 
               i, pids[i], WEXITSTATUS(status));
    }
    
    double end = get_time_ms();
    printf("[Parent] All workers completed. Total time: %.2f ms\n", end - start);
}

void test_fork_snapshot() {
    printf("\n=== Test 6: Snapshot Pattern (P3 Pattern - Simplified) ===\n");
    printf("Pattern: P3 - Fork for state snapshot (NO exec)\n");
    
    // Simulate some important data
    int important_data = 42;
    printf("[Parent] Important data before fork: %d\n", important_data);
    
    double start = get_time_ms();
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child process - reads snapshot
        printf("[Child Snapshot] Reading snapshot data: %d\n", important_data);
        printf("[Child Snapshot] This simulates taking a snapshot\n");
        printf("[Child Snapshot] In real use (like Redis), child would save to disk\n");
        sleep(1); // Simulate snapshot save time
        printf("[Child Snapshot] Snapshot saved successfully\n");
        exit(0);
    } else {
        // Parent process - continues with modified data
        printf("[Parent] Snapshot process started (PID: %d)\n", pid);
        printf("[Parent] Continuing work while snapshot is being taken...\n");
        
        // Parent modifies data while child is snapshotting
        important_data = 100;
        printf("[Parent] Modified data to: %d (child has old snapshot)\n", important_data);
        
        int status;
        waitpid(pid, &status, 0);
        double end = get_time_ms();
        printf("[Parent] Snapshot completed with status %d\n", WEXITSTATUS(status));
        printf("[Parent] Time taken: %.2f ms\n", end - start);
    }
}

void print_statistics() {
    printf("\n=== Spork Statistics ===\n");
    printf("Tests Run: 6\n");
    printf("  - P1 (fork+exec): 3 tests\n");
    printf("  - P2 (workers):   2 tests\n");
    printf("  - P3 (snapshot):  1 test\n");
    printf("\nExpected behavior:\n");
    printf("  - P1 tests should use Facade (posix_spawn)\n");
    printf("  - P2/P3 tests should use uFork (fallback to real fork)\n");
    printf("  - In production, P1 should be ~3-10x faster\n");
}

int main() {
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║         Spork Test Suite - Comprehensive             ║\n");
    printf("║  Testing all fork patterns from the paper            ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    
    // Run all tests
    test_simple_fork_exit();
    test_fork_exec();
    test_fork_exec_with_args();
    test_worker_process();
    test_multiple_workers();
    test_fork_snapshot();
    
    // Print summary
    print_statistics();
    
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║              All Tests Completed                     ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    
    return 0;
}
