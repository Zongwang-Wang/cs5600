# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -D_GNU_SOURCE
LDFLAGS = -ldl

SPORK_OBJS = spork.o facade.o ufork.o primer.o

all: libspork.so primer test_spork

libspork.so: $(SPORK_OBJS)
	$(CC) -shared -o libspork.so $(SPORK_OBJS) $(LDFLAGS)

primer: primer_loader.c spork.h
	$(CC) $(CFLAGS) -o primer primer_loader.c

test_spork: test_spork.c libspork.so spork.h
	$(CC) $(CFLAGS) -o test_spork test_spork.c -L. -lspork -Wl,-rpath,.

spork.o: spork.c spork.h
	$(CC) $(CFLAGS) -c spork.c -o spork.o

facade.o: facade.c spork.h
	$(CC) $(CFLAGS) -c facade.c -o facade.o

ufork.o: ufork.c spork.h
	$(CC) $(CFLAGS) -c ufork.c -o ufork.o

primer.o: primer.c spork.h
	$(CC) $(CFLAGS) -c primer.c -o primer.o

clean:
	rm -f *.o *.so primer test_spork /tmp/spork_primer_*

run: all
	LD_PRELOAD=./libspork.so ./test_spork

.PHONY: all clean run
