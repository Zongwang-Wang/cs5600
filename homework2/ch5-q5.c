#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        printf("Child: I'm running\n");
        sleep(2);
        printf("Child: I'm done\n");
        return 0;
    } else {
        // Parent process
        printf("Parent: Waiting for child\n");
        wait(NULL);
        printf("Parent: Child finished\n");
    }
    
    return 0;
}
