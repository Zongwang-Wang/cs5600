// primer_loader.c - The actual Primer executable
#include "spork.h"

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
    read(fd, &num_changes, sizeof(int));
    
    process_state_change_t *changes = calloc(num_changes, 
                                             sizeof(process_state_change_t));
    read(fd, changes, num_changes * sizeof(process_state_change_t));
    
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
            // Add more cases as needed
            default:
                break;
        }
    }
    
    // Read target path
    size_t path_len;
    read(fd, &path_len, sizeof(size_t));
    char *target_path = malloc(path_len);
    read(fd, target_path, path_len);
    
    // Read target argv
    int target_argc;
    read(fd, &target_argc, sizeof(int));
    char **target_argv = calloc(target_argc + 1, sizeof(char*));
    for (int i = 0; i < target_argc; i++) {
        size_t arg_len;
        read(fd, &arg_len, sizeof(size_t));
        target_argv[i] = malloc(arg_len);
        read(fd, target_argv[i], arg_len);
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