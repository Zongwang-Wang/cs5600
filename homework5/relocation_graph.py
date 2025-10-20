#! /usr/bin/env python3

from __future__ import print_function
import random
import math
import matplotlib.pyplot as plt


# --- Utility functions ---
def random_seed(seed):
    try:
        random.seed(seed, version=1)
    except:
        random.seed(seed)


def convert(size):
    """Convert sizes like '1k', '4m', '32g' to integer bytes."""
    size = str(size)
    lastchar = size[-1]
    if lastchar in ['k', 'K']:
        return int(size[:-1]) * 1024
    elif lastchar in ['m', 'M']:
        return int(size[:-1]) * 1024 * 1024
    elif lastchar in ['g', 'G']:
        return int(size[:-1]) * 1024 * 1024 * 1024
    else:
        return int(size)


# --- Simulation: generate random virtual addresses, check which are valid ---
def run_segmentation(seed, asize, psize, limit, num_addrs=1000):
    random_seed(seed)
    valid = 0
    total = num_addrs

    # pick base randomly such that base + limit <= psize
    while True:
        base = int(psize * random.random())
        if (base + limit) < psize:
            break

    # generate random virtual addresses
    for _ in range(num_addrs):
        vaddr = int(asize * random.random())
        if vaddr < limit:  # valid address
            valid += 1

    return valid / total  # fraction valid


# --- Experiment: sweep through different limit values ---
def main():
    asize = convert('1k')     # virtual address space
    psize = convert('16k')    # physical memory size
    seeds = list(range(5))    # number of random seeds
    limits = list(range(0, asize + 1, asize // 20))  # 0 → asize in 20 steps

    results = []
    for limit in limits:
        avg_fraction_valid = 0
        for seed in seeds:
            frac = run_segmentation(seed, asize, psize, limit)
            avg_fraction_valid += frac
        avg_fraction_valid /= len(seeds)
        results.append((limit, avg_fraction_valid))

    # --- Plot Results ---
    limits_bytes = [r[0] for r in results]
    fractions_valid = [r[1] for r in results]

    plt.figure(figsize=(8, 5))
    plt.plot(limits_bytes, fractions_valid, marker='o', color='green')
    plt.title('Fraction of Valid Virtual Addresses vs Limit Register')
    plt.xlabel('Limit Register Value (bytes)')
    plt.ylabel('Fraction of Valid Addresses')
    plt.grid(True)
    plt.ylim(0, 1.05)
    plt.show()


if __name__ == '__main__':
    main()
