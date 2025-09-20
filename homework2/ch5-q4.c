#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    
    // Test execl()
    pid = fork();
    if (pid == 0) {
        execl("/bin/ls", "ls", "-l", NULL);
        perror("execl failed");
        return 1;
    }
    wait(NULL);
    
    // Test execle()
    pid = fork();
    if (pid == 0) {
        char *env[] = {NULL};
        execle("/bin/ls", "ls", "-a", NULL, env);
        perror("execle failed");
        return 1;
    }
    wait(NULL);
    
    // Test execlp()
    pid = fork();
    if (pid == 0) {
        execlp("ls", "ls", "-F", NULL);
        perror("execlp failed");
        return 1;
    }
    wait(NULL);
    
    // Test execv()
    pid = fork();
    if (pid == 0) {
        char *args[] = {"ls", "-la", NULL};
        execv("/bin/ls", args);
        perror("execv failed");
        return 1;
    }
    wait(NULL);
    
    // Test execvp()
    pid = fork();
    if (pid == 0) {
        char *args[] = {"ls", "-h", NULL};
        execvp("ls", args);
        perror("execvp failed");
        return 1;
    }
    wait(NULL);
    
    // Test execvpe() (may not work on all systems)
    pid = fork();
    if (pid == 0) {
        char *args[] = {"ls", "-1", NULL};
        char *env[] = {NULL};
        execvpe("ls", args, env);
        perror("execvpe failed");
        return 1;
    }
    wait(NULL);
    
    printf("\nWhy so many variants?\n");
    printf("- 'l' = arguments as list, 'v' = arguments as array\n");
    printf("- 'p' = search PATH, no 'p' = need full path\n");
    printf("- 'e' = custom environment, no 'e' = inherit environment\n");
    
    return 0;
}