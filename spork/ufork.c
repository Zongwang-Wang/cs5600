// ufork.c - Real fork fallback
#include "spork.h"
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>

// uFork: Actually use real fork for P2/P3
pid_t ufork_clone_process(void) {
    printf("[uFork] Falling back to REAL fork() - Full process duplication\n");
    
    // Get original fork function
    pid_t (*real_fork)(void) = dlsym(RTLD_NEXT, "fork");
    if (!real_fork) {
        fprintf(stderr, "[uFork] ERROR: Cannot find real fork\n");
        errno = ENOSYS;
        return -1;
    }
    
    printf("[uFork] Calling real fork()...\n");
    pid_t result = real_fork();
    
    if (result == 0) {
        printf("[uFork] In child process (PID: %d)\n", getpid());
    } else if (result > 0) {
        printf("[uFork] Parent: Created child PID: %d\n", result);
    } else {
        printf("[uFork] Fork failed: %s\n", strerror(errno));
    }
    
    return result;
}