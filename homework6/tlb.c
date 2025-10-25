#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>

#define PAGESIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_pages> <trials>\n", argv[0]);
        exit(1);
    }

    int num_pages = atoi(argv[1]);
    long long trials = atoll(argv[2]);

    // --- Q6: Pin to a single CPU (CPU 0) ---
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0) {
        perror("sched_setaffinity");
        // not fatal; just continue
    }

    int jump = PAGESIZE / sizeof(int);
    int *a = malloc((size_t)num_pages * PAGESIZE);
    if (a == NULL) {
        perror("malloc");
        exit(1);
    }

    // Initialize each page once to avoid page faults
    for (int i = 0; i < num_pages * jump; i += jump)
        a[i] = 0;

    struct timeval start, end;

    // --- Q5: Prevent compiler optimization ---
    // Use 'volatile' so compiler cannot remove the loop
    volatile int sink = 0;

    gettimeofday(&start, NULL);

    for (long long t = 0; t < trials; t++) {
        for (int i = 0; i < num_pages * jump; i += jump) {
            a[i] += 1;
            sink += a[i];  // ensure side effect is visible
        }
    }

    gettimeofday(&end, NULL);

    double elapsed_us = (end.tv_sec - start.tv_sec) * 1e6 +
                        (end.tv_usec - start.tv_usec);
    double per_access_ns = (elapsed_us * 1000) / (num_pages * trials);

    printf("%d pages, %.2f ns per access\n", num_pages, per_access_ns);

    // print sink so compiler really can't skip it
    if (sink == 0x12345678) printf("sink=%d\n", sink);

    free((void *)a);
    return 0;
}