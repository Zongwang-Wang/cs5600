#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        printf("Child: This will be printed\n");
        
        // Close standard output
        close(STDOUT_FILENO);
        
        // This won't be visible
        printf("Child: This won't be printed\n");
        
        // But we can still write to stderr
        fprintf(stderr, "Child: This goes to stderr\n");
        
        return 0;
    } else {
        // Parent process
        wait(NULL);
        printf("Parent: Child finished\n");
    }
    
    return 0;
}
