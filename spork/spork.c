// spork.c - Main implementation
#include "spork.h"
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

// Global context for current fork operation
static fork_context_t *current_fork_ctx = NULL;

// Original fork function pointer (for fallback)
static pid_t (*original_fork)(void) = NULL;

// Initialize Spork
void __attribute__((constructor)) spork_init(void) {
    // Get pointer to original fork for fallback
    original_fork = dlsym(RTLD_NEXT, "fork");
    if (!original_fork) {
        fprintf(stderr, "Spork: Failed to find original fork\n");
    }
}

// Create a new fork context
static fork_context_t *create_fork_context(void) {
    fork_context_t *ctx = calloc(1, sizeof(fork_context_t));
    if (!ctx) {
        return NULL;
    }
    ctx->pattern = PATTERN_FORK_EXEC;
    ctx->exec_found = false;
    ctx->state_changes = NULL;
    ctx->num_changes = 0;
    return ctx;
}

// Free fork context
static void free_fork_context(fork_context_t *ctx) {
    if (!ctx) return;
    
    if (ctx->exec_path) free(ctx->exec_path);
    
    if (ctx->exec_argv) {
        for (int i = 0; ctx->exec_argv[i]; i++) {
            free(ctx->exec_argv[i]);
        }
        free(ctx->exec_argv);
    }
    
    if (ctx->state_changes) {
        for (int i = 0; i < ctx->num_changes; i++) {
            if (ctx->state_changes[i].action == ACTION_SETENV) {
                free(ctx->state_changes[i].data.setenv.name);
                free(ctx->state_changes[i].data.setenv.value);
            } else if (ctx->state_changes[i].action == ACTION_CHDIR) {
                free(ctx->state_changes[i].data.chdir.path);
            }
        }
        free(ctx->state_changes);
    }
    
    free(ctx);
}

// Add a state change to the context
static int add_state_change(fork_context_t *ctx, process_state_change_t change) {
    process_state_change_t *new_changes = realloc(
        ctx->state_changes,
        (ctx->num_changes + 1) * sizeof(process_state_change_t)
    );
    
    if (!new_changes) {
        return -1;
    }
    
    ctx->state_changes = new_changes;
    ctx->state_changes[ctx->num_changes] = change;
    ctx->num_changes++;
    
    return 0;
}

// Spork's main fork replacement
pid_t spork_fork(void) {
    printf("[Spork] fork() called, analyzing...\n");
    
    // Create context for this fork
    fork_context_t *ctx = create_fork_context();
    if (!ctx) {
        errno = ENOMEM;
        return -1;
    }
    
    current_fork_ctx = ctx;
    
    // Try Facade: Analyze code path to find exec
    // In a real implementation, this would use symbolic execution or emulation
    // For this prototype, we'll simulate the analysis
    
    // Simulate: Check if exec will be called (simplified heuristic)
    // In practice, this would analyze the call stack and instruction flow
    
    // For demonstration, assume fork+exec pattern 84% of the time
    // In reality, Facade would perform actual code analysis
    
    printf("[Facade] Analyzing fork-exec pattern...\n");
    
    // Simulate finding exec call and parameters
    // This is where symbolic execution or emulation would happen
    ctx->exec_found = true; // Simulated
    ctx->exec_path = strdup("/bin/echo");
    
    char *argv[] = {"echo", "Hello from Spork!", NULL};
    ctx->exec_argv = calloc(3, sizeof(char*));
    ctx->exec_argv[0] = strdup(argv[0]);
    ctx->exec_argv[1] = strdup(argv[1]);
    ctx->exec_argv[2] = NULL;
    
    if (ctx->exec_found) {
        // Facade found exec - use posix_spawn
        printf("[Facade] Found exec, using posix_spawn\n");
        pid_t pid = facade_analyze_and_spawn(ctx);
        free_fork_context(ctx);
        current_fork_ctx = NULL;
        return pid;
    } else {
        // Facade didn't find exec - fall back to uFork
        printf("[uFork] No exec found, cloning process\n");
        pid_t pid = ufork_clone_process();
        free_fork_context(ctx);
        current_fork_ctx = NULL;
        return pid;
    }
}

// Override fork symbol
pid_t fork(void) {
    return spork_fork();
}