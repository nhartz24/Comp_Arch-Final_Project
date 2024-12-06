import matplotlib.pyplot as plt
import numpy as np

# Define the two datasets as lists of strings
dataset1 = [
    "17516444295 17459652609 17488819614 17523030140 17437698477",
    "36246409801 36238644820 36316405664 36339011349 36489455289",
    "75529732929 75467489699 75696597578 75777383951 75735737038",
    "156157685607 156433536913 156181124561 156213936872 156072957943",
    "323034861506 322342587056 322672753907 322204990327 321966205194",
    "668139308900 668512886541 668729968449 668684178904 668008001174"
]

dataset2 = [
    "15490965718 15507797683 15439778284 15538105237 15468747256",
    "31484638727 31449683892 31353787119 31424579134 31369350450",
    "65448746177 65283144945 65724916793 65315312562 65304274136",
    "135679030102 135795145770 135612939882 135429887029 135620156871",
    "282336883143 281318906428 281316496510 281223890307 280823483084",
    "582778018820 582959920564 582609200442 583766534358 583272193301",
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

# Compute speedup
speedup = [(a1 / a2) *1e11 for a1, a2 in zip(averages1, averages2)]

# Array sizes and labels
array_sizes = [2**25, 2**26, 2**27, 2**28, 2**29, 2**30]
x_labels = ['2^25', '2^26', '2^27', '2^28', '2^29', '2^30']

# Create a figure with one subplot
fig, ax = plt.subplots(figsize=(8, 6))

# Plot the median ticks for Dynamic and Static
ax.plot(x_labels, averages1, label="Normal", marker="o")
ax.plot(x_labels, averages2, label="Tile", marker="o")

# Plot the speedup on the same graph, using a secondary axis
ax2 = ax.twinx()  # Create a second y-axis
ax2.plot(x_labels, speedup, label="Speedup (Normal \ Tile)", marker="o", color="green")



# Set titles and labels
ax.set_title("Normal vs. Tiled MergeSort")
ax.set_xlabel("Array Size")
ax.set_ylabel("Average Ticks (100 bil)")
ax2.set_ylabel("Speedup")

# Set the range of the second y-axis to match the range of the first one
ax2.set_ylim(ax.get_ylim())

# Add legends
ax.legend(loc="upper left")
ax2.legend(loc="upper right")

# Add grid lines for clarity
ax.grid(True)

# Show and save the plot
plt.tight_layout()
plt.savefig("graph_with_overlayed_speedup.png")
# plt.show()
