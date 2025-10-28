// spork.c - Simplified (no state_changes)
#include "spork.h"
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>


// Pattern hint from application
static fork_pattern_t next_fork_pattern = PATTERN_AUTO_DETECT;
static char *next_exec_path = NULL;
static char **next_exec_argv = NULL;
static char **next_exec_envp = NULL;

// Original fork function pointer (for fallback)
static pid_t (*original_fork)(void) = NULL;

// Initialize Spork
void __attribute__((constructor)) spork_init(void) {
    original_fork = dlsym(RTLD_NEXT, "fork");
    if (!original_fork) {
        fprintf(stderr, "Spork: Failed to find original fork\n");
    }
    printf("[Spork] Library loaded and initialized\n");
}

// API: Let application hint the pattern
void spork_set_next_fork_pattern(fork_pattern_t pattern) {
    next_fork_pattern = pattern;
    printf("[Spork API] Pattern hint set: %d\n", pattern);
}

// API: Let application set exec parameters
void spork_set_next_exec_params(const char *path, char *const argv[], char *const envp[]) {
    if (next_exec_path) free(next_exec_path);
    next_exec_path = strdup(path);
    
    // Copy argv
    int argc = 0;
    if (argv) {
        while (argv[argc]) argc++;
        next_exec_argv = calloc(argc + 1, sizeof(char*));
        for (int i = 0; i < argc; i++) {
            next_exec_argv[i] = strdup(argv[i]);
        }
    }
    
    // Copy envp (or use NULL for default environment)
    next_exec_envp = envp ? (char**)envp : NULL;
    
    printf("[Spork API] Exec params set: %s with %d args\n", path, argc);
}

// Create a new fork context
static fork_context_t *create_fork_context(void) {
    fork_context_t *ctx = calloc(1, sizeof(fork_context_t));
    if (!ctx) {
        return NULL;
    }
    ctx->pattern = PATTERN_AUTO_DETECT;
    ctx->exec_found = false;
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
    
    free(ctx);
}

// Clear the hints after use
static void clear_hints(void) {
    next_fork_pattern = PATTERN_AUTO_DETECT;
    if (next_exec_path) {
        free(next_exec_path);
        next_exec_path = NULL;
    }
    if (next_exec_argv) {
        for (int i = 0; next_exec_argv[i]; i++) {
            free(next_exec_argv[i]);
        }
        free(next_exec_argv);
        next_exec_argv = NULL;
    }
    next_exec_envp = NULL;
}

// Spork's main fork replacement
pid_t spork_fork(void) {
    printf("[Spork] fork() intercepted\n");
    
    // Create context for this fork
    fork_context_t *ctx = create_fork_context();
    if (!ctx) {
        errno = ENOMEM;
        return -1;
    }
    
    // Use pattern hint if provided
    if (next_fork_pattern != PATTERN_AUTO_DETECT) {
        ctx->pattern = next_fork_pattern;
        printf("[Spork] Using explicit pattern: %d\n", ctx->pattern);
    }
    
    // Use exec params if provided
    if (next_exec_path) {
        ctx->exec_found = true;
        ctx->exec_path = strdup(next_exec_path);
        
        // Copy argv
        if (next_exec_argv) {
            int argc = 0;
            while (next_exec_argv[argc]) argc++;
            ctx->exec_argv = calloc(argc + 1, sizeof(char*));
            for (int i = 0; i < argc; i++) {
                ctx->exec_argv[i] = strdup(next_exec_argv[i]);
            }
            ctx->exec_argv[argc] = NULL;
        }
        
        ctx->exec_envp = next_exec_envp;
    }
    
    pid_t result;
    
    // Decision based on pattern
    if (ctx->pattern == PATTERN_FORK_EXEC || ctx->exec_found) {
        // P1: Use Facade
        printf("[Spork] P1 Pattern detected: fork+exec → Using Facade\n");
        result = facade_analyze_and_spawn(ctx);
    } else {
        // P2/P3: Use uFork
        printf("[Spork] P2/P3 Pattern detected: No exec → Using uFork\n");
        result = ufork_clone_process();
    }
    
    free_fork_context(ctx);
    clear_hints();
    
    return result;
}

// Override fork symbol
pid_t fork(void) {
    return spork_fork();
}