// spork.h - Main header file
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
    PATTERN_SNAPSHOT = 3       // P3: process snapshots
} fork_pattern_t;

// Process state modification actions
typedef enum {
    ACTION_NONE = 0,
    ACTION_SIGNAL_HANDLER,
    ACTION_CLOSE_FD,
    ACTION_DUP_FD,
    ACTION_SETENV,
    ACTION_CHDIR,
    ACTION_SETUID,
    ACTION_SETGID
} primer_action_t;

// Structure to hold process state changes
typedef struct {
    primer_action_t action;
    union {
        struct {
            int signum;
            void (*handler)(int);
        } signal;
        struct {
            int fd;
        } close_fd;
        struct {
            int oldfd;
            int newfd;
        } dup_fd;
        struct {
            char *name;
            char *value;
        } setenv;
        struct {
            char *path;
        } chdir;
        struct {
            uid_t uid;
        } setuid;
        struct {
            gid_t gid;
        } setgid;
    } data;
} process_state_change_t;

// Structure to track fork-exec patterns
typedef struct {
    pid_t pid;
    fork_pattern_t pattern;
    char *exec_path;
    char **exec_argv;
    char **exec_envp;
    process_state_change_t *state_changes;
    int num_changes;
    bool exec_found;
} fork_context_t;

// Function prototypes
pid_t spork_fork(void);
int facade_analyze_and_spawn(fork_context_t *ctx);
int primer_execute(const char *target_path, char *const argv[], 
                   char *const envp[], process_state_change_t *changes, 
                   int num_changes);
pid_t ufork_clone_process(void);

#endif // SPORK_H