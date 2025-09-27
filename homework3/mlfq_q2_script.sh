#!/bin/bash

# MLFQ Chapter 8 Question 2
# Reproduce each example from the chapter

echo "=========================================="
echo "MLFQ Scheduler - Question 2"
echo "Reproducing examples from Chapter 8"
echo "=========================================="
echo

# Example 1: A Single Long-Running Job (Figure 8.2)
echo "=========================================="
echo "Example 1: Single Long-Running Job (Figure 8.2)"
echo "=========================================="
echo "A single job that runs for 200ms, moving down through 3 queues"
echo
./mlfq.py -n 3 -q 10 -l 0,200,0 -c
echo
echo "Press Enter to continue..."
read

# Example 2: Along Came A Short Job (Figure 8.3 left)
echo "=========================================="
echo "Example 2: Along Came A Short Job (Figure 8.3 left)"
echo "=========================================="
echo "Long job A (180ms) runs, then short job B (20ms) arrives at time 100"
echo
./mlfq.py -n 3 -q 10 -l 0,180,0:100,20,0 -c
echo
echo "Press Enter to continue..."
read

# Example 3: What About I/O? (Figure 8.3 right)
echo "=========================================="
echo "Example 3: What About I/O? (Figure 8.3 right)"  
echo "=========================================="
echo "Job B does I/O every 1ms, staying at high priority"
echo
./mlfq.py -n 3 -q 10 -l 0,180,0:100,20,1 -i 5 -c
echo
echo "Press Enter to continue..."
read

# Example 4a: Without Priority Boost (Figure 8.4 left)
echo "=========================================="
echo "Example 4a: Without Priority Boost (Figure 8.4 left)"
echo "=========================================="
echo "Long job can starve when interactive jobs arrive"
echo
./mlfq.py -n 3 -q 10 -l 0,180,0:100,20,1:100,20,1 -i 5 -c
echo
echo "Press Enter to continue..."
read

# Example 4b: With Priority Boost (Figure 8.4 right)
echo "=========================================="
echo "Example 4b: With Priority Boost (Figure 8.4 right)"
echo "=========================================="
echo "Priority boost every 100ms prevents starvation"
echo
./mlfq.py -n 3 -q 10 -l 0,180,0:100,20,1:100,20,1 -i 5 -B 100 -c
echo
echo "Press Enter to continue..."
read

# Example 5a: Gaming with Old Rules (Figure 8.5 left)
echo "=========================================="
echo "Example 5a: Gaming with Old Rules 4a/4b (Figure 8.5 left)"
echo "=========================================="
echo "Job 1 games the scheduler by doing I/O every 9ms with -S flag"
echo
./mlfq.py -n 3 -q 10 -l 0,200,0:0,100,9 -i 1 -S -c
echo
echo "Press Enter to continue..."
read

# Example 5b: Better Accounting (Figure 8.5 right)
echo "=========================================="
echo "Example 5b: Better Accounting Prevents Gaming (Figure 8.5 right)"
echo "=========================================="
echo "New Rule 4 tracks total CPU usage, preventing gaming"
echo
./mlfq.py -n 3 -q 10 -l 0,200,0:0,100,9 -i 1 -c
echo
echo "Press Enter to continue..."
read

# Example 6: Different Time Slices (Figure 8.6)
echo "=========================================="
echo "Example 6: Different Time Slices per Queue (Figure 8.6)"
echo "=========================================="
echo "Queue time slices: Q2=10ms, Q1=20ms, Q0=40ms"
echo
./mlfq.py -n 3 -Q 10,20,40 -l 0,200,0:50,50,0 -c
echo

echo "=========================================="
echo "Question 2 Complete!"
echo "All chapter examples have been reproduced"
echo "=========================================="