#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {
    
    int fd = open("output.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process writes to the file
        const char *msg = "Child writing...\n";
        write(fd, msg, 17);
    } else {
        // Parent process writes to the file
        const char *msg = "Parent writing...\n";
        write(fd, msg, 18);
        wait(NULL);
    }

    close(fd);
    return 0;
}