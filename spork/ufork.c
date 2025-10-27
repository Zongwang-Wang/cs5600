// ufork.c - uFork implementation (fallback) - corrected
#include "spork.h"
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>

// uFork: Clone process in user space
pid_t ufork_clone_process(void) {
    printf("[uFork] Creating full process clone\n");
    
    // In a real implementation, this would:
    // 1. Use posix_spawn to create a new process
    // 2. Use Primer to copy all memory from parent to child
    // 3. Use Primer to set up all process state
    // 4. Have Primer return instead of exec
    
    // For this prototype, we'll fall back to real fork
    // In production, this would be a full user-space implementation
    
    printf("[uFork] Warning: Using real fork as fallback\n");
    
    // Get original fork function
    pid_t (*real_fork)(void) = dlsym(RTLD_NEXT, "fork");
    if (!real_fork) {
        errno = ENOSYS;
        return -1;
    }
    
    return real_fork();
}
