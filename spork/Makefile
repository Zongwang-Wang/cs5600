# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -D_GNU_SOURCE
LDFLAGS = -ldl

all: libspork.so primer test_spork

libspork.so: spork.o facade.o ufork.o primer.o
	$(CC) -shared -o $@ $^ $(LDFLAGS)

primer: primer_loader.c
	$(CC) $(CFLAGS) -o $@ $

test_spork: test_spork.c libspork.so
	$(CC) $(CFLAGS) -o $@ test_spork.c -L. -lspork -Wl,-rpath,.

%.o: %.c spork.h
	$(CC) $(CFLAGS) -c $

clean:
	rm -f *.o *.so primer test_spork

run: all
	LD_PRELOAD=./libspork.so ./test_spork

.PHONY: all clean run