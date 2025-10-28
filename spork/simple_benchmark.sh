#!/bin/bash
# simple_benchmark.sh - Quick side-by-side comparison

echo "Building programs..."
make clean > /dev/null 2>&1
make all > /dev/null 2>&1
gcc -O2 -o benchmark benchmark.c

echo ""
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║              QUICK BENCHMARK: Spork vs Traditional           ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Test 1: Simple timing
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TEST 1: Simple Timing (50 iterations)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "[Traditional fork()]"
time ./benchmark 50 2>&1 | grep "Average"

echo ""
echo "[Spork]"
LD_PRELOAD=./libspork.so time ./test_spork 2>&1 | grep "P1.*average"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TEST 2: Memory Usage (with /usr/bin/time -v)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "[Traditional fork() - Memory Stats]"
/usr/bin/time -v ./benchmark 20 2>&1 | grep -E "(Maximum resident|Voluntary context)"

echo ""
echo "[Spork - Memory Stats]"
LD_PRELOAD=./libspork.so /usr/bin/time -v ./test_spork 2>&1 | grep -E "(Maximum resident|Voluntary context)"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TEST 3: System Call Counts (strace -c)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "[Traditional fork() - syscall summary]"
strace -c ./benchmark 10 2>&1 | tail -20 | head -10

echo ""
echo "[Spork - syscall summary]"
LD_PRELOAD=./libspork.so strace -c ./test_spork 2>&1 | tail -20 | head -10

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TEST 4: Detailed Fork/Exec Syscalls"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "[Traditional: fork + execve pattern (first 5 iterations)]"
strace -e trace=fork,clone,vfork,execve ./benchmark 5 2>&1 | grep -E "(clone|fork|execve)" | head -15

echo ""
echo "[Spork: should use posix_spawn (first 5 iterations)]"
LD_PRELOAD=./libspork.so strace -e trace=fork,clone,vfork,execve,posix_spawn ./test_spork 2>&1 | grep -E "(clone|fork|execve|posix_spawn)" | head -15

echo ""
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║                    BENCHMARK COMPLETE                         ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""
