# Makefile - With benchmarking support
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -D_GNU_SOURCE
LDFLAGS = -ldl

SPORK_OBJS = spork.o facade.o ufork.o

all: libspork.so test_spork benchmark

libspork.so: $(SPORK_OBJS)
	$(CC) -shared -o libspork.so $(SPORK_OBJS) $(LDFLAGS)

test_spork: test_spork.c libspork.so spork.h
	$(CC) $(CFLAGS) -o test_spork test_spork.c -L. -lspork -Wl,-rpath,.

benchmark: benchmark.c
	$(CC) $(CFLAGS) -o benchmark benchmark.c

spork.o: spork.c spork.h
	$(CC) $(CFLAGS) -c spork.c -o spork.o

facade.o: facade.c spork.h
	$(CC) $(CFLAGS) -c facade.c -o facade.o

ufork.o: ufork.c spork.h
	$(CC) $(CFLAGS) -c ufork.c -o ufork.o

clean:
	rm -f *.o *.so test_spork benchmark
	rm -rf benchmark_results

run: all
	@echo "=========================================="
	@echo "Running Spork Test Suite"
	@echo "=========================================="
	@LD_PRELOAD=./libspork.so ./test_spork

# Run comprehensive benchmarks
bench: all run_benchmarks.sh
	@chmod +x run_benchmarks.sh
	@./run_benchmarks.sh

# Run quick side-by-side comparison
quick-bench: all simple_benchmark.sh
	@chmod +x simple_benchmark.sh
	@./simple_benchmark.sh

.PHONY: all clean run bench quick-bench