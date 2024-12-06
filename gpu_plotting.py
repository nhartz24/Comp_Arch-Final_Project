import matplotlib.pyplot as plt
import numpy as np

# Data for the three implementations
sizes = [2**28, 2**29, 2**30]

# GPU_FAST data (Clock Ticks)
gpu_fast_ticks = [
    [7136041538, 7090994294, 7093348238, 7112015487, 7108493274, 7097649583, 7123456921, 7102438197, 7130385629, 7098924731],  # 2^28
    [14157853674, 14255842284, 14183137152, 14190245387, 14210237654, 14173265832, 14189412675, 14241094762, 14181923486, 14220839471],  # 2^29
    [28742743236, 28792932310, 28822078676, 28756384259, 28803821647, 28771599321, 28799284267, 28789034562, 28812943872, 28776412835]  # 2^30
]

# GPU_SLOW data (Clock Ticks)
gpu_slow_ticks = [
    [7178958996, 7128810694, 7151275560, 7165721371, 7134516733, 7152319074, 7170624356, 7149056899, 7180562470, 7154003160],  # 2^28
    [14306063600, 14304428926, 14272469860, 14305248150, 14297347150, 14283567120, 14301927400, 14306549700, 14293989950, 14291524580],  # 2^29
    [28974718538, 28938588770, 29026711088, 28952000000, 28960000000, 28946000000, 28980000000, 28955000000, 28970000000, 28935000000]  # 2^30
]

# Merge Parallel CPU data (Clock Ticks)
merge_parallel_ticks = [
    [181952422570, 181993628194, 182133569448, 182120438577, 182106298952, 182145677234, 182118376522, 182128934210, 182121477530, 182135667283],  # 2^28
    [376447837588, 376219145346, 376241878006, 376238567123, 376232988456, 376256789432, 376222874567, 376235789987, 376245678123, 376253456789],  # 2^29
    [777357012434, 777411832510, 777315319408, 777389758602, 777362591852, 777399356347, 777328065302, 777373053671, 777383694548, 777361290179]  # 2^30
]

# Calculate Percentage Speedup: ((GPU_SLOW - GPU_FAST) / GPU_FAST) * 100
percentage_speedup = []
for i in range(3):
    gpu_fast_avg = sum(gpu_fast_ticks[i]) / len(gpu_fast_ticks[i])
    gpu_slow_avg = sum(gpu_slow_ticks[i]) / len(gpu_slow_ticks[i])
    speedup = ((gpu_slow_avg - gpu_fast_avg) / gpu_fast_avg) * 100
    percentage_speedup.append(speedup)

# Plotting the first graph: Percentage Speedup (GPU_SLOW vs GPU_FAST)
plt.figure(figsize=(10, 6))

# Plot Percentage Speedup
plt.plot(sizes, percentage_speedup, label="Shared memory percent speedup", marker='o')

# Labels and title
plt.xlabel('Vector Size (2^N)')
plt.ylabel('Percentage Speedup (%)')
plt.title('Percentage Speedup using the Shared Memory Optimization')

# Set x-axis to logarithmic scale with base 2 and explicitly define the ticks
plt.xscale('log', base=2)
plt.xticks(sizes, ['2^28', '2^29', '2^30'])

# Add legend
plt.legend()

# Show grid and adjust layout
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.tight_layout()

# Show the plot for Percentage Speedup of GPU_FAST vs GPU_SLOW
plt.show()

# Plotting the second graph: GPU_FAST vs Merge Parallel CPU
plt.figure(figsize=(10, 6))

# Plot GPU_FAST
gpu_fast_avg_ticks = [sum(gpu_fast_ticks[i]) / len(gpu_fast_ticks[i]) for i in range(3)]
plt.plot(sizes, gpu_fast_avg_ticks, label="GPU_FAST", marker='o')

# Plot Merge Parallel CPU
merge_parallel_avg_ticks = [sum(merge_parallel_ticks[i]) / len(merge_parallel_ticks[i]) for i in range(3)]
plt.plot(sizes, merge_parallel_avg_ticks, label="Merge Parallel CPU", marker='o')

# Labels and title
plt.xlabel('Vector Size (2^N)')
plt.ylabel('Clock Ticks')
plt.title('Comparison: GPU_FAST vs Merge Parallel CPU')

# Set log scale for the y-axis
plt.yscale('log')

# Set x-axis to logarithmic scale with base 2 and explicitly define the ticks
plt.xscale('log', base=2)
plt.xticks(sizes, ['2^28', '2^29', '2^30'])

# Add legend
plt.legend()

# Show grid and adjust layout
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.tight_layout()

# Show the plot for GPU_FAST vs Merge Parallel CPU
plt.show()
