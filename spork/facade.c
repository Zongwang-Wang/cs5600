// facade.c - Simplified implementation (no Primer)
#include "spork.h"
#include <sys/stat.h>
#include <errno.h>

extern char **environ;

// Facade: Directly spawn the process with posix_spawn
int facade_analyze_and_spawn(fork_context_t *ctx) {
    if (!ctx) {
        errno = EINVAL;
        return -1;
    }
    
    // If no exec params provided, we can't proceed
    if (!ctx->exec_path) {
        printf("[Facade] ERROR: No exec path provided for P1 pattern\n");
        errno = EINVAL;
        return -1;
    }
    
    printf("[Facade] Spawning process with posix_spawn\n");
    printf("[Facade]   Program: %s\n", ctx->exec_path);
    if (ctx->exec_argv && ctx->exec_argv[0]) {
        printf("[Facade]   Args: ");
        for (int i = 0; ctx->exec_argv[i]; i++) {
            printf("%s ", ctx->exec_argv[i]);
        }
        printf("\n");
    }
    
    pid_t pid;
    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attr;
    
    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_init(&attr);
    
    // Use provided environment or default
    char **envp = ctx->exec_envp ? ctx->exec_envp : environ;
    
    int ret = posix_spawn(&pid, ctx->exec_path, &actions, &attr,
                         ctx->exec_argv, envp);
    
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attr);
    
    if (ret != 0) {
        printf("[Facade] posix_spawn failed: %s\n", strerror(ret));
        errno = ret;
        return -1;
    }
    
    printf("[Facade] Successfully spawned child PID: %d\n", pid);
    return pid;
}