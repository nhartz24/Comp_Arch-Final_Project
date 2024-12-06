import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Read both CSV files
simd_df = pd.read_csv('simd_results.csv')
vanilla_df = pd.read_csv('vanilla_results.csv')

# Calculate medians for each size
simd_medians = simd_df.groupby('power').agg({
    'cycles': 'median'
}).reset_index()
vanilla_medians = vanilla_df.groupby('power').agg({
    'cycles': 'median'
}).reset_index()

# Calculate speedup
medians = pd.merge(simd_medians, vanilla_medians, on='power', suffixes=('_simd', '_vanilla'))
medians['speedup'] = medians['cycles_vanilla'] / medians['cycles_simd']

# Create bar plot for cycles (log scale)
plt.figure(figsize=(8, 6))
plt.rcParams.update({'font.size': 14})
x = range(len(medians))
width = 0.35

plt.bar(x, medians['cycles_simd'], width, label='SIMD')
plt.bar([i + width for i in x], medians['cycles_vanilla'], width, label='Vanilla')

plt.xlabel('Array Size (2^n)', fontsize=16)
plt.ylabel('Median Cycles', fontsize=16)
plt.title('Sorting Performance Comparison', fontsize=18)
plt.xticks([i + width/2 for i in x], [f'2^{power}' for power in medians['power']])
plt.legend()
plt.yscale('log')
plt.savefig('performance_comparison.png')
plt.close()

# Create non-logarithmic bar plot for cycles
plt.figure(figsize=(8, 6))
x = range(len(medians))
width = 0.35

plt.bar(x, medians['cycles_simd'], width, label='SIMD')
plt.bar([i + width for i in x], medians['cycles_vanilla'], width, label='Vanilla')

plt.xlabel('Array Size (2^n)', fontsize=16)
plt.ylabel('Median Cycles', fontsize=16)
plt.title('Sorting Performance Comparison (Linear Scale)', fontsize=18)
plt.xticks([i + width/2 for i in x], [f'2^{power}' for power in medians['power']])
plt.legend()
plt.savefig('performance_comparison_linear.png')
plt.close()

# Create speedup plot
plt.figure(figsize=(8, 6))
plt.plot(medians['power'], medians['speedup'], marker='o')
plt.xlabel('Array Size (2^n)', fontsize=16)
plt.ylabel('Speedup (x faster)', fontsize=16)
plt.title('SIMD Speedup vs Array Size', fontsize=18)
plt.grid(True)
plt.savefig('speedup.png')
plt.close()