#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("hello\n");
    } else {
        // Parent process
        sleep(1);   // give child a chance to run first
        printf("goodbye\n");
    }

    return 0;
}