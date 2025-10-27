// primer.c - Primer implementation
#include "spork.h"

#define PRIMER_PATH "./primer"

// Primer: Execute with process state modifications
int primer_execute(const char *target_path, char *const argv[], 
                   char *const envp[], process_state_change_t *changes, 
                   int num_changes) {
    
    printf("[Primer] Setting up process state and loading %s\n", target_path);
    
    // Create a temporary file to pass state changes to Primer
    char tmp_file[] = "/tmp/spork_primer_XXXXXX";
    int fd = mkstemp(tmp_file);
    if (fd < 0) {
        perror("mkstemp");
        return -1;
    }
    
    // Write state changes to file
    write(fd, &num_changes, sizeof(int));
    write(fd, changes, num_changes * sizeof(process_state_change_t));
    
    // Write target path length and path
    size_t path_len = strlen(target_path) + 1;
    write(fd, &path_len, sizeof(size_t));
    write(fd, target_path, path_len);
    
    // Write argv
    int argc = 0;
    while (argv[argc]) argc++;
    write(fd, &argc, sizeof(int));
    for (int i = 0; i < argc; i++) {
        size_t arg_len = strlen(argv[i]) + 1;
        write(fd, &arg_len, sizeof(size_t));
        write(fd, argv[i], arg_len);
    }
    
    close(fd);
    
    // Spawn Primer with posix_spawn
    pid_t pid;
    char *primer_argv[] = {PRIMER_PATH, tmp_file, NULL};
    
    int ret = posix_spawn(&pid, PRIMER_PATH, NULL, NULL, primer_argv, envp);
    
    // Clean up temp file
    unlink(tmp_file);
    
    if (ret != 0) {
        errno = ret;
        return -1;
    }
    
    return pid;
}