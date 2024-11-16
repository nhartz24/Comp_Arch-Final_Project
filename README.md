# compile
gcc -O3 -mavx2 -o radix_sort_compare r_simd_vs_vanilla.c


er3211@instance-20240925-160652:~/homework4$ ./radix_sort_compare 

Sorting complete.
SIMD sort time: 68233700 cycles
Vanilla sort time: 685272178 cycles
Percentage speedup: 90.04%
Both sorts validated successfully. 
