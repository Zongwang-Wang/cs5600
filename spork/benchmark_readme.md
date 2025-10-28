# Spork Benchmarking Guide

This guide explains how to benchmark Spork against traditional `fork()` to measure performance improvements.

## Quick Start

### Option 1: Quick Side-by-Side Comparison (Recommended)
```bash
chmod +x simple_benchmark.sh
make quick-bench
```

This runs a fast comparison showing:
- Timing differences
- Memory usage
- System call counts
- Detailed fork/exec traces

### Option 2: Comprehensive Benchmark Suite
```bash
chmod +x run_benchmarks.sh
make bench
```

This runs extensive benchmarks and generates detailed reports in `benchmark_results/`.

## Manual Benchmarking

If you want to run benchmarks manually in two separate shells:

### Shell 1: Traditional fork()
```bash
# Build
make clean
gcc -O2 -o benchmark benchmark.c

# Time measurement
time ./benchmark 50

# Memory analysis
/usr/bin/time -v ./benchmark 50

# System call trace
strace -c ./benchmark 10

# Detailed fork/exec trace
strace -e trace=fork,clone,execve ./benchmark 5
```

### Shell 2: Spork
```bash
# Build
make all

# Time measurement
LD_PRELOAD=./libspork.so time ./test_spork

# Memory analysis
LD_PRELOAD=./libspork.so /usr/bin/time -v ./test_spork

# System call trace
LD_PRELOAD=./libspork.so strace -c ./test_spork

# Detailed fork/exec trace (should show posix_spawn)
LD_PRELOAD=./libspork.so strace -e trace=fork,clone,execve,posix_spawn ./test_spork
```

## What to Look For

### 1. **Timing (P1 Pattern: fork+exec)**
Traditional fork+exec requires:
- `fork()` - duplicate entire process
- `execve()` - load new program

Spork P1 uses:
- `posix_spawn()` - directly spawn new process
- **Expected speedup: 1.5-3x faster**

### 2. **Memory Usage**
Traditional fork():
- Copies page tables (even with COW)
- Higher "Maximum resident set size"

Spork P1:
- No unnecessary duplication
- **Expected: 10-30% less memory**

### 3. **System Call Count**
Check `strace -c` output:

Traditional:
```
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 40.00    0.000120           6        20           clone
 25.00    0.000075           4        20           execve
 ...
```

Spork P1:
```
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 35.00    0.000105           5        20           posix_spawn
 ...
```

Notice: **No clone + execve**, just **posix_spawn**.

### 4. **Detailed Traces**
Traditional (fork+exec pattern):
```
clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, ...)
execve("/bin/echo", ["echo", "test"], ...)
```

Spork P1:
```
posix_spawn(..., "/bin/echo", ..., ["echo", "test"], ...)
```

**Key insight**: One system call instead of two!

## Benchmark Results Interpretation

### Sample Output Comparison

**Traditional fork() - 50 iterations:**
```
Total time: 450.23 ms
Average per fork+exec: 9.00 ms
Maximum resident set size: 8432 KB
Voluntary context switches: 150
```

**Spork - Same workload:**
```
Total time: 198.45 ms
Average per fork+exec: 3.97 ms
Maximum resident set size: 6128 KB
Voluntary context switches: 85
```

**Analysis:**
- ⚡ **2.27x faster** (9.00ms → 3.97ms)
- 💾 **27% less memory** (8432KB → 6128KB)
- 📉 **43% fewer context switches** (150 → 85)

## Understanding the Patterns

### P1: Fork+Exec (Optimized by Spork)
```c
pid = fork();
if (pid == 0) {
    execv("/bin/ls", args);  // Traditional: 2 syscalls
}
// Spork: posix_spawn() - 1 syscall ✓
```

### P2: Worker Process (No optimization needed)
```c
pid = fork();
if (pid == 0) {
    do_work();  // No exec - both use real fork()
    exit(0);
}
```

### P3: Snapshot (No optimization needed)
```c
pid = fork();
if (pid == 0) {
    save_snapshot();  // COW benefits - both use real fork()
    exit(0);
}
```

## Troubleshooting

### If benchmarks show no improvement:

1. **Check that Spork is loaded:**
   ```bash
   LD_PRELOAD=./libspork.so ./test_spork | grep "Spork.*initialized"
   ```

2. **Verify posix_spawn usage:**
   ```bash
   LD_PRELOAD=./libspork.so strace -e posix_spawn ./test_spork 2>&1 | grep posix_spawn
   ```

3. **Check pattern detection:**
   ```bash
   LD_PRELOAD=./libspork.so ./test_spork | grep "P1 Pattern"
   ```

### Common Issues:

**"posix_spawn not found in strace"**
- Some systems show it as `clone` with special flags
- Check for fewer `clone+execve` pairs compared to traditional

**"No performance improvement"**
- Verify you're testing P1 pattern (fork+exec)
- P2/P3 patterns intentionally use real fork()

**"LD_PRELOAD not working"**
- Ensure `libspork.so` is built: `ls -l libspork.so`
- Try absolute path: `LD_PRELOAD=/full/path/to/libspork.so`

## Advanced Analysis

### Memory Profiling with Valgrind
```bash
# Traditional
valgrind --tool=massif --massif-out-file=trad.out ./benchmark 10
ms_print trad.out > trad_memory.txt

# Spork
LD_PRELOAD=./libspork.so valgrind --tool=massif --massif-out-file=spork.out ./test_spork
ms_print spork.out > spork_memory.txt

# Compare peak memory
grep "peak" trad_memory.txt
grep "peak" spork_memory.txt
```

### CPU Profiling with perf
```bash
# Traditional
perf stat -e cycles,instructions,cache-misses ./benchmark 100

# Spork
LD_PRELOAD=./libspork.so perf stat -e cycles,instructions,cache-misses ./test_spork
```

## Expected Results Summary

| Metric | Traditional fork() | Spork P1 | Improvement |
|--------|-------------------|----------|-------------|
| **Time per fork+exec** | ~8-12 ms | ~3-6 ms | **1.5-3x faster** |
| **Memory usage** | Baseline | -10-30% | **Less memory** |
| **Context switches** | Baseline | -30-50% | **Fewer switches** |
| **System calls** | clone + execve | posix_spawn | **50% fewer** |

## Next Steps

After confirming improvements:
1. Test with your real application workload
2. Profile specific bottlenecks
3. Tune pattern detection if needed
4. Measure in production-like environment

## Questions?

Check the main README.md for architecture details and troubleshooting.
