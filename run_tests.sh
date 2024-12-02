#!/bin/bash

# Compile the program
gcc -O3 -mavx2 -o radix_sort_simd radix_simd_vs_vanilla.c

# Create CSV file with headers
echo "power,size,simd_cycles,vanilla_cycles,speedup" > results.csv

# Test sizes from 2^8 to 2^22
for power in {8..22}; do
    echo "Testing size 2^$power"
    # Run 100 times for each size
    for run in {1..100}; do
        ./radix_sort_simd $power >> results.csv
    done
done
