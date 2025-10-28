#!/bin/bash
# run_benchmarks.sh - Comprehensive benchmarking script

ITERATIONS=50
RESULTS_DIR="benchmark_results"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║         SPORK vs TRADITIONAL FORK BENCHMARK SUITE            ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Create results directory
mkdir -p "$RESULTS_DIR"

# Clean previous builds
echo -e "${YELLOW}[1/6] Cleaning previous builds...${NC}"
make clean > /dev/null 2>&1

# Build everything
echo -e "${YELLOW}[2/6] Building Spork library...${NC}"
make all
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to build Spork${NC}"
    exit 1
fi

echo -e "${YELLOW}[3/6] Building traditional fork benchmark...${NC}"
gcc -Wall -Wextra -O2 -o benchmark benchmark.c
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to build benchmark${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}                    BENCHMARK 1: TIME COMPARISON${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo ""

# Benchmark 1: Traditional fork with time
echo -e "${BLUE}[4/6] Running traditional fork() benchmark...${NC}"
/usr/bin/time -v ./benchmark $ITERATIONS 2> "$RESULTS_DIR/traditional_time.txt" > "$RESULTS_DIR/traditional_output.txt"

echo ""
echo -e "${BLUE}[5/6] Running Spork benchmark...${NC}"
LD_PRELOAD=./libspork.so /usr/bin/time -v ./test_spork 2> "$RESULTS_DIR/spork_time.txt" > "$RESULTS_DIR/spork_output.txt"

echo ""
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}                    BENCHMARK 2: STRACE ANALYSIS${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo ""

# Benchmark 2: System call analysis with strace
echo -e "${BLUE}[6/6] Running strace analysis...${NC}"

# Traditional fork - count system calls
echo -e "${YELLOW}Analyzing traditional fork() syscalls...${NC}"
strace -c -o "$RESULTS_DIR/traditional_strace.txt" ./benchmark 10 > /dev/null 2>&1

# Spork - count system calls
echo -e "${YELLOW}Analyzing Spork syscalls...${NC}"
LD_PRELOAD=./libspork.so strace -c -o "$RESULTS_DIR/spork_strace.txt" ./test_spork > /dev/null 2>&1

# Detailed strace for fork+exec pattern
echo -e "${YELLOW}Detailed syscall trace (fork+exec)...${NC}"
strace -e trace=fork,clone,vfork,execve,posix_spawn -o "$RESULTS_DIR/traditional_detailed.txt" ./benchmark 5 > /dev/null 2>&1
LD_PRELOAD=./libspork.so strace -e trace=fork,clone,vfork,execve,posix_spawn -o "$RESULTS_DIR/spork_detailed.txt" ./test_spork > /dev/null 2>&1

echo ""
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}                         RESULTS SUMMARY${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo ""

# Extract and display key metrics
echo -e "${BLUE}=== TIME METRICS ===${NC}"
echo ""
echo -e "${YELLOW}Traditional Fork:${NC}"
grep "Elapsed" "$RESULTS_DIR/traditional_time.txt"
grep "Maximum resident" "$RESULTS_DIR/traditional_time.txt"
grep "Voluntary context" "$RESULTS_DIR/traditional_time.txt"

echo ""
echo -e "${YELLOW}Spork:${NC}"
grep "Elapsed" "$RESULTS_DIR/spork_time.txt"
grep "Maximum resident" "$RESULTS_DIR/spork_time.txt"
grep "Voluntary context" "$RESULTS_DIR/spork_time.txt"

echo ""
echo -e "${BLUE}=== SYSTEM CALL COUNTS ===${NC}"
echo ""
echo -e "${YELLOW}Traditional Fork (top syscalls):${NC}"
head -20 "$RESULTS_DIR/traditional_strace.txt"

echo ""
echo -e "${YELLOW}Spork (top syscalls):${NC}"
head -20 "$RESULTS_DIR/spork_strace.txt"

echo ""
echo -e "${BLUE}=== FORK/EXEC PATTERN DETAILS ===${NC}"
echo ""
echo -e "${YELLOW}Traditional Fork calls:${NC}"
grep -E "(fork|clone|execve)" "$RESULTS_DIR/traditional_detailed.txt" | head -10

echo ""
echo -e "${YELLOW}Spork calls (should show posix_spawn):${NC}"
grep -E "(fork|clone|execve|posix_spawn)" "$RESULTS_DIR/spork_detailed.txt" | head -10

echo ""
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}           All results saved to: $RESULTS_DIR/${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo ""

# Generate comparison report
cat > "$RESULTS_DIR/comparison_report.txt" << EOF
SPORK vs TRADITIONAL FORK() BENCHMARK REPORT
Generated: $(date)
Iterations: $ITERATIONS

═══════════════════════════════════════════════════════════════

TIMING COMPARISON
─────────────────────────────────────────────────────────────

Traditional Fork:
$(grep "Elapsed" "$RESULTS_DIR/traditional_time.txt")

Spork:
$(grep "Elapsed" "$RESULTS_DIR/spork_time.txt")

═══════════════════════════════════════════════════════════════

MEMORY USAGE
─────────────────────────────────────────────────────────────

Traditional Fork:
$(grep "Maximum resident" "$RESULTS_DIR/traditional_time.txt")

Spork:
$(grep "Maximum resident" "$RESULTS_DIR/spork_time.txt")

═══════════════════════════════════════════════════════════════

CONTEXT SWITCHES
─────────────────────────────────────────────────────────────

Traditional Fork:
$(grep "Voluntary context" "$RESULTS_DIR/traditional_time.txt")
$(grep "Involuntary context" "$RESULTS_DIR/traditional_time.txt")

Spork:
$(grep "Voluntary context" "$RESULTS_DIR/spork_time.txt")
$(grep "Involuntary context" "$RESULTS_DIR/spork_time.txt")

═══════════════════════════════════════════════════════════════

For detailed system call traces, see:
  - $RESULTS_DIR/traditional_strace.txt
  - $RESULTS_DIR/spork_strace.txt
  - $RESULTS_DIR/traditional_detailed.txt
  - $RESULTS_DIR/spork_detailed.txt

EOF

echo -e "${GREEN}Summary report written to: $RESULTS_DIR/comparison_report.txt${NC}"
echo ""
