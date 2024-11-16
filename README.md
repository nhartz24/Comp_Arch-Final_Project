# compile
gcc -O3 -mavx2 -o radix_sort_compare r_simd_vs_vanilla.c


er3211@instance-20240925-160652:~/homework4$ ./radix_sort_compare 

Sorting complete.
SIMD sort time: 68233700 cycles
Vanilla sort time: 685272178 cycles
Percentage speedup: 90.04%
Both sorts validated successfully. 



questions for the TAs:
- changinging processors on GCP (how are we supposed to be optimizing, RAM/cores/money???)
- confirm that we should be optimizing through SIMD and parallelizing? is our idea good
- are we ultimately suppoosed to use both sorting implemtations or just one (prof said chose two)
- should we show speed up over time as we add optimizations?


