#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <megabytes>\n", argv[0]);
        return 1;
    }

    int mb = atoi(argv[1]);
    size_t size = (size_t)mb * 1024 * 1024;
    char *mem = malloc(size);
    if (!mem) {
        printf("Memory allocation failed\n");
        return 1;
    }

    printf("Allocated %d MB. Using it continuously...\n", mb);
    while (1) {
        for (size_t i = 0; i < size; i += 4096)
            mem[i] = 0;  // touch each page
        usleep(10000);  // small pause
    }

    free(mem);
    return 0;
}