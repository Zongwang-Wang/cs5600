// primer_loader.c - The actual Primer executable (corrected)
#include "spork.h"
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Primer: No state file provided\n");
        return 1;
    }
    
    const char *state_file = argv[1];
    
    // Read state changes from file
    int fd = open(state_file, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    int num_changes;
    if (read(fd, &num_changes, sizeof(int)) < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    
    process_state_change_t *changes = calloc(num_changes, 
                                             sizeof(process_state_change_t));
    if (!changes) {
        perror("calloc");
        close(fd);
        return 1;
    }
    
    if (read(fd, changes, num_changes * sizeof(process_state_change_t)) < 0) {
        perror("read");
        free(changes);
        close(fd);
        return 1;
    }
    
    // Apply state changes
    printf("[Primer] Applying %d state changes\n", num_changes);
    for (int i = 0; i < num_changes; i++) {
        switch (changes[i].action) {
            case ACTION_CLOSE_FD:
                close(changes[i].data.close_fd.fd);
                break;
            case ACTION_DUP_FD:
                dup2(changes[i].data.dup_fd.oldfd, changes[i].data.dup_fd.newfd);
                break;
            case ACTION_SIGNAL_HANDLER:
                signal(changes[i].data.signal.signum, 
                       changes[i].data.signal.handler);
                break;
            case ACTION_NONE:
            case ACTION_SETENV:
            case ACTION_CHDIR:
            case ACTION_SETUID:
            case ACTION_SETGID:
            default:
                break;
        }
    }
    
    free(changes);
    
    // Read target path
    size_t path_len;
    if (read(fd, &path_len, sizeof(size_t)) < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    
    char *target_path = malloc(path_len);
    if (!target_path) {
        perror("malloc");
        close(fd);
        return 1;
    }
    
    if (read(fd, target_path, path_len) < 0) {
        perror("read");
        free(target_path);
        close(fd);
        return 1;
    }
    
    // Read target argv
    int target_argc;
    if (read(fd, &target_argc, sizeof(int)) < 0) {
        perror("read");
        free(target_path);
        close(fd);
        return 1;
    }
    
    char **target_argv = calloc(target_argc + 1, sizeof(char*));
    if (!target_argv) {
        perror("calloc");
        free(target_path);
        close(fd);
        return 1;
    }
    
    for (int i = 0; i < target_argc; i++) {
        size_t arg_len;
        if (read(fd, &arg_len, sizeof(size_t)) < 0) {
            perror("read");
            for (int j = 0; j < i; j++) free(target_argv[j]);
            free(target_argv);
            free(target_path);
            close(fd);
            return 1;
        }
        
        target_argv[i] = malloc(arg_len);
        if (!target_argv[i]) {
            perror("malloc");
            for (int j = 0; j < i; j++) free(target_argv[j]);
            free(target_argv);
            free(target_path);
            close(fd);
            return 1;
        }
        
        if (read(fd, target_argv[i], arg_len) < 0) {
            perror("read");
            for (int j = 0; j <= i; j++) free(target_argv[j]);
            free(target_argv);
            free(target_path);
            close(fd);
            return 1;
        }
    }
    target_argv[target_argc] = NULL;
    
    close(fd);
    
    // Execute target program
    printf("[Primer] Executing %s\n", target_path);
    execv(target_path, target_argv);
    
    // If we get here, exec failed
    perror("execv");
    return 1;
}
