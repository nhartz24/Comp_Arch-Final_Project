import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import ScalarFormatter

# Define the two datasets as lists of strings
dataset1 = [
    "19412157373 19317654356 19337741979 19385126866 19292995425",
    "40175826057 40207239958 40154549008 40799556956 40034710149",
    "83826401512 82831171920 83019662964 83065354165 83106560740",
    "172184350660 171026034956 171028641756 170988693306 170950236611",
    "356544615868 356010551368 356074624013 356236480324 355374744690",
    "731514079191 727956871589 729494151039 730361503690 729839800102"
]

dataset2 = [
    "19556939902 19541333305 19586592154 19325497321 19444895645",
    "40463355877 40248048211 40350898525 40508993350 40386921683",
    "83778600224 83430117246 83429452488 83141758346 83511743284",
    "172957949381 172311988310 172092196899 172338625249 172172986164",
    "355780149375 355179057188 353342321293 351288990920 352491121419",
    "726661881852 725450151762 723743258665 725101112267 726416791104"
]

def calculate_averages(dataset):
    averages = []
    for line in dataset:
        numbers = list(map(int, line.split()))
        averages.append(sum(numbers) / len(numbers))
    return averages

# Compute averages
averages1 = calculate_averages(dataset1)
averages2 = calculate_averages(dataset2)

# # Generate x-axis points (1, 2, 3, ...)
# x_points = range(1, len(averages1) + 1)

array_sizes = [2**25, 2**26, 2**27, 2**28, 2**29, 2**30]
x_labels = ['2^25', '2^26', '2^27', '2^28', '2^29', '2^30']

# averages1 = [y / 1e9 for y in averages1]
# averages2 = [y / 1e9 for y in averages2]

# Plot the data
plt.plot(x_labels, averages1, label="Recursive", marker="o")
plt.plot(x_labels, averages2, label="Iterative", marker="o")

# Customize the plot
plt.title("Recurive vs. Iteative MergeSort")
plt.xlabel("Array Size")
plt.ylabel("Average Ticks (100 bil)")
plt.legend()
plt.grid(True)

# Show the plot
plt.savefig("graph.png")