#!/bin/bash

TRIALS=100000      # large enough for stable timing
MAXPAGES=4096        # test up to 4096 pages
CPU=0                # pin process to CPU 0

# Recompile with -O0 and volatile to prevent optimization 
echo "Compiling tlb.c with optimization disabled..."
gcc -O0 -Wall -std=c11 -o tlb tlb.c

echo "Pages, ns_per_access"

for ((P=1; P<=MAXPAGES; P*=2)); do
    ./tlb $P $TRIALS | awk '{print $1 "," $3}'
done