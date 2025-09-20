#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int x = 100;
    printf("Before fork, x = %d\n", x);

    pid_t pid = fork();

    if (pid < 0) {
        // fork failed
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child: initially x = %d\n", x);
        x = 200;
        printf("Child: changed x to %d\n", x);
    } else {
        // Parent process
        wait(NULL); // wait for child to finish
        printf("Parent: initially x = %d\n", x);
        x = 300;
        printf("Parent: changed x to %d\n", x);
    }

    return 0;
}