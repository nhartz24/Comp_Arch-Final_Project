#!/bin/bash

# Compile both programs
gcc -O3 -mavx2 -o radix_sort_simd radix_sorting_simd.c
gcc -O3 -o radix_sort_vanilla radix_sorting_vanilla.c

# Create CSV files with headers
echo "power,size,cycles" > simd_results.csv
echo "power,size,cycles" > vanilla_results.csv

# Test sizes from 2^20 to 2^30, incrementing by 2
for power in {20..30..2}; do
    echo "Testing size 2^$power"
    # Run 10 times for each size
    for run in {1..10}; do
        ./radix_sort_simd $power >> simd_results.csv
        ./radix_sort_vanilla $power >> vanilla_results.csv
    done
done
