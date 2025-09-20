#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t pid1, pid2;
    
    // Create pipe
    pipe(pipefd);
    
    // First child - writes to pipe
    pid1 = fork();
    if (pid1 == 0) {
        close(pipefd[0]);  // Close read end
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe
        close(pipefd[1]);
        
        printf("Hello from first child!\n");
        return 0;
    }
    
    // Second child - reads from pipe
    pid2 = fork();
    if (pid2 == 0) {
        close(pipefd[1]);  // Close write end
        dup2(pipefd[0], STDIN_FILENO);  // Redirect stdin from pipe
        close(pipefd[0]);
        
        char buffer[100];
        fgets(buffer, sizeof(buffer), stdin);
        printf("Second child received: %s", buffer);
        return 0;
    }
    
    // Parent
    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
    
    return 0;
}
