// spork.h - Simplified header (no Primer)
#ifndef SPORK_H
#define SPORK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

// Fork pattern classification
typedef enum {
    PATTERN_FORK_EXEC = 1,    // P1: fork followed by exec
    PATTERN_WORKER = 2,        // P2: worker processes
    PATTERN_SNAPSHOT = 3,      // P3: process snapshots
    PATTERN_AUTO_DETECT = 0    // Let Spork decide
} fork_pattern_t;

// Structure to track fork-exec patterns
typedef struct {
    pid_t pid;
    fork_pattern_t pattern;
    char *exec_path;
    char **exec_argv;
    char **exec_envp;
    bool exec_found;
} fork_context_t;

// API for applications to hint pattern to Spork
void spork_set_next_fork_pattern(fork_pattern_t pattern);
void spork_set_next_exec_params(const char *path, char *const argv[], char *const envp[]);

// Function prototypes
pid_t spork_fork(void);
int facade_analyze_and_spawn(fork_context_t *ctx);
pid_t ufork_clone_process(void);

#endif // SPORK_H