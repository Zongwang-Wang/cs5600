#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <spawn.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_LINE 2048

extern char **environ;

// Statistics
typedef struct {
    long total_spawns;
    long fork_calls;
    long posix_spawn_calls;
    long total_time_us;
    long optimized_count;
    long analyzed_count;
} ShellStats;

ShellStats stats = {0};

long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int parse_command(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return i;
}

// Analyze source code for fork+exec pattern
int analyze_source_for_exec(const char *source_path) {
    FILE *fp = fopen(source_path, "r");
    if (!fp) return 0;
    
    char line[MAX_LINE];
    int found_exec = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "execl(") || strstr(line, "execv(") || 
            strstr(line, "execve(") || strstr(line, "execvp(")) {
            found_exec = 1;
            printf("    Found: %s", line);
            break;
        }
    }
    
    fclose(fp);
    return found_exec;
}

void cmd_stats() {
    printf("\n=== SPORK-SHELL STATISTICS ===\n");
    printf("Total spawns:           %ld\n", stats.total_spawns);
    printf("  fork() calls:         %ld\n", stats.fork_calls);
    printf("  posix_spawn() calls:  %ld\n", stats.posix_spawn_calls);
    printf("\n");
    printf("Total execution time:   %ld microseconds (%.3f ms)\n", 
           stats.total_time_us, stats.total_time_us / 1000.0);
    if (stats.total_spawns > 0) {
        printf("Average time per spawn: %ld microseconds\n", 
               stats.total_time_us / stats.total_spawns);
    }
    printf("\n");
    printf("Binaries analyzed:      %ld\n", stats.analyzed_count);
    printf("Optimized (posix_spawn):%ld (%.1f%%)\n", 
           stats.optimized_count,
           stats.analyzed_count > 0 ? 
           (stats.optimized_count * 100.0 / stats.analyzed_count) : 0);
    printf("==============================\n\n");
}

void cmd_reset() {
    memset(&stats, 0, sizeof(stats));
    printf("[RESET] Statistics cleared\n");
}

void cmd_help() {
    printf("\n=== SPORK-SHELL ===\n");
    printf("Analyzes source, executes binary with optimization\n\n");
    printf("Commands:\n");
    printf("  stats  - Show statistics\n");
    printf("  reset  - Clear stats\n");
    printf("  help   - This help\n");
    printf("  exit   - Exit\n");
    printf("\nUsage:\n");
    printf("  Pass .c file: analyzes source, runs binary\n");
    printf("  Example: spork-shell> test_fork_exec.c\n");
    printf("           → Analyzes test_fork_exec.c\n");
    printf("           → Executes ./test_fork_exec\n");
    printf("===========================\n\n");
}

// Rewrite source code to use posix_spawn instead of fork+exec
int rewrite_to_posix_spawn(const char *source_path, const char *output_path) {
    FILE *in = fopen(source_path, "r");
    FILE *out = fopen(output_path, "w");
    
    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        return 0;
    }
    
    char line[MAX_LINE];
    int in_fork_section = 0;
    int brace_count = 0;
    
    // Add necessary includes at the top
    fprintf(out, "#include <spawn.h>\n");
    fprintf(out, "extern char **environ;\n\n");
    
    while (fgets(line, sizeof(line), in)) {
        // Skip original spawn.h includes
        if (strstr(line, "#include") && strstr(line, "spawn.h")) {
            continue;
        }
        
        // Detect fork() call - start of optimization
        if (strstr(line, "fork()")) {
            fprintf(out, "    // SPORK OPTIMIZED: fork() replaced with posix_spawn\n");
            fprintf(out, "    pid_t pid;\n");
            in_fork_section = 1;
            continue;
        }
        
        // Inside fork section, look for exec
        if (in_fork_section && (strstr(line, "execl(") || strstr(line, "execv("))) {
            // Extract exec path
            char *exec_pos = strstr(line, "execl(");
            if (!exec_pos) exec_pos = strstr(line, "execv(");
            
            if (exec_pos) {
                char *quote1 = strchr(exec_pos, '"');
                if (quote1) {
                    quote1++;
                    char *quote2 = strchr(quote1, '"');
                    if (quote2) {
                        char path[256];
                        int len = quote2 - quote1;
                        strncpy(path, quote1, len);
                        path[len] = '\0';
                        
                        fprintf(out, "    char *args[] = {\"%s\", NULL};\n", path);
                        fprintf(out, "    posix_spawn(&pid, \"%s\", NULL, NULL, args, environ);\n", path);
                    }
                }
            }
            continue;
        }
        
        // Skip _exit in child after exec
        if (in_fork_section && (strstr(line, "_exit") || strstr(line, "perror"))) {
            continue;
        }
        
        // Track braces to know when fork section ends
        if (in_fork_section) {
            for (int i = 0; line[i]; i++) {
                if (line[i] == '{') brace_count++;
                if (line[i] == '}') {
                    brace_count--;
                    if (brace_count <= 0) {
                        in_fork_section = 0;
                    }
                }
            }
        }
        
        // Write line as-is if not in optimization section
        if (!in_fork_section || brace_count > 1) {
            fprintf(out, "%s", line);
        }
    }
    
    fclose(in);
    fclose(out);
    return 1;
}

int execute_command(char **args) {
    if (!args[0]) return 0;
    
    // Must be a .c file
    if (!strstr(args[0], ".c")) {
        printf("Error: Only .c files supported\n");
        return 1;
    }
    
    long start = get_time_us();
    
    // Get base name without .c
    char basename[256];
    strncpy(basename, args[0], sizeof(basename) - 1);
    char *dot = strstr(basename, ".c");
    if (dot) *dot = '\0';
    
    char executable[256];
    snprintf(executable, sizeof(executable), "./%s", basename);
    
    // Analyze source
    printf("[ANALYZING] %s... ", args[0]);
    fflush(stdout);
    
    stats.analyzed_count++;
    int has_exec = analyze_source_for_exec(args[0]);
    
    char source_to_compile[256];
    
    if (has_exec) {
        printf("✓ exec() found\n");
        printf("[REWRITING] Replacing fork+exec with posix_spawn...\n");
        stats.optimized_count++;
        
        // Create optimized source
        snprintf(source_to_compile, sizeof(source_to_compile), 
                 "/tmp/spork_opt_%s.c", basename);
        
        if (!rewrite_to_posix_spawn(args[0], source_to_compile)) {
            printf("[ERROR] Failed to rewrite source\n");
            return 1;
        }
        
        printf("[REWRITTEN] ✓ Created: %s\n", source_to_compile);
        stats.posix_spawn_calls++;
    } else {
        printf("✗ No exec() found\n");
        strcpy(source_to_compile, args[0]);
        stats.fork_calls++;
    }
    
    // Compile
    printf("[COMPILING] gcc %s -o %s...\n", source_to_compile, executable);
    
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd), 
             "gcc %s -o %s 2>&1", source_to_compile, executable);
    
    FILE *compile_out = popen(compile_cmd, "r");
    if (compile_out) {
        char line[256];
        int has_error = 0;
        while (fgets(line, sizeof(line), compile_out)) {
            printf("  %s", line);
            has_error = 1;
        }
        pclose(compile_out);
        
        if (has_error) {
            // Check if executable was still created
            if (access(executable, X_OK) != 0) {
                printf("[ERROR] Compilation failed\n");
                return 1;
            }
        }
    }
    
    printf("[COMPILED] ✓\n");
    
    // Execute
    printf("[EXECUTING] %s\n", executable);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        execl(executable, executable, NULL);
        perror("exec");
        exit(1);
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    long end = get_time_us();
    stats.total_spawns++;
    stats.total_time_us += (end - start);
    
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int is_builtin(char *cmd) {
    return (strcmp(cmd, "stats") == 0 ||
            strcmp(cmd, "reset") == 0 ||
            strcmp(cmd, "help") == 0 ||
            strcmp(cmd, "exit") == 0);
}

int execute_builtin(char **args) {
    if (strcmp(args[0], "stats") == 0) {
        cmd_stats();
    } else if (strcmp(args[0], "reset") == 0) {
        cmd_reset();
    } else if (strcmp(args[0], "help") == 0) {
        cmd_help();
    } else if (strcmp(args[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
    return 0;
}

void shell_loop() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    
    while (1) {
        printf("spork-shell> ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }
        
        if (input[0] == '\n') continue;
        
        int argc = parse_command(input, args);
        if (argc == 0) continue;
        
        if (is_builtin(args[0])) {
            execute_builtin(args);
        } else {
            execute_command(args);
        }
    }
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║   SPORK-SHELL v1.0 (MVP)               ║\n");
    printf("║   Source analysis + Binary execution   ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");
    printf("Analyzes .c source, executes binary\n");
    printf("Type 'help' for commands\n\n");
    
    shell_loop();
    
    printf("\nFinal stats:\n");
    cmd_stats();
    
    return 0;
}
