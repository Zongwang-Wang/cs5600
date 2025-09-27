#!/bin/bash

# MLFQ Chapter 8 Question 1
# Run randomly-generated problems with 2 jobs, 2 queues
# Limited job length, no I/O

echo "=========================================="
echo "MLFQ Scheduler - Question 1"
echo "Running random problems with 2 jobs, 2 queues"
echo "Job length limited to 20, I/O disabled"
echo "=========================================="
echo

# Run 5 different random seeds to see various scenarios
for seed in 1 2 3 4 5
do
    echo "=========================================="
    echo "Run #$seed (seed=$seed)"
    echo "=========================================="
    
    # Run without solution first to show the problem
    echo "--- Problem Definition ---"
    ./mlfq.py -j 2 -n 2 -m 20 -M 0 -s $seed
    
    echo
    echo "--- Execution Trace and Solution ---"
    # Run with -c flag to show the execution trace
    ./mlfq.py -j 2 -n 2 -m 20 -M 0 -s $seed -c
    
    echo
    echo "Press Enter to continue to next example..."
    read
done

echo "=========================================="
echo "Question 1 Complete!"
echo "Examined 5 random MLFQ scenarios with 2 jobs and 2 queues"
echo "=========================================="