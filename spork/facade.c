// facade.c - Facade implementation
#include "spork.h"
#include <sys/stat.h>

extern char **environ;

// Facade: Analyze and spawn process
int facade_analyze_and_spawn(fork_context_t *ctx) {
    if (!ctx || !ctx->exec_path) {
        errno = EINVAL;
        return -1;
    }
    
    // Determine if we need Primer for complex state changes
    bool needs_primer = (ctx->num_changes > 0);
    
    if (!needs_primer) {
        // Simple case: directly use posix_spawn
        printf("[Facade] Simple fork-exec, using posix_spawn directly\n");
        
        pid_t pid;
        posix_spawn_file_actions_t actions;
        posix_spawnattr_t attr;
        
        posix_spawn_file_actions_init(&actions);
        posix_spawnattr_init(&attr);
        
        int ret = posix_spawn(&pid, ctx->exec_path, &actions, &attr,
                             ctx->exec_argv, environ);
        
        posix_spawn_file_actions_destroy(&actions);
        posix_spawnattr_destroy(&attr);
        
        if (ret != 0) {
            errno = ret;
            return -1;
        }
        
        return pid;
    } else {
        // Complex case: use Primer
        printf("[Facade] Complex state changes, using Primer\n");
        return primer_execute(ctx->exec_path, ctx->exec_argv, environ,
                            ctx->state_changes, ctx->num_changes);
    }
}