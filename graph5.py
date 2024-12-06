import matplotlib.pyplot as plt
import numpy as np

# Define the datasets as lists of strings (you already have these)
dataset1 = [
    "19412157373 19317654356 19337741979 19385126866 19292995425",
    "40175826057 40207239958 40154549008 40799556956 40034710149",
    "83826401512 82831171920 83019662964 83065354165 83106560740",
    "172184350660 171026034956 171028641756 170988693306 170950236611",
    "356544615868 356010551368 356074624013 356236480324 355374744690",
    "731514079191 727956871589 729494151039 730361503690 729839800102"
]

dataset2 = [
    "17516444295 17459652609 17488819614 17523030140 17437698477",
    "36246409801 36238644820 36316405664 36339011349 36489455289",
    "75529732929 75467489699 75696597578 75777383951 75735737038",
    "156157685607 156433536913 156181124561 156213936872 156072957943",
    "323034861506 322342587056 322672753907 322204990327 321966205194",
    "668139308900 668512886541 668729968449 668684178904 668008001174"
]

dataset3 = [
    "15490965718 15507797683 15439778284 15538105237 15468747256",
    "31484638727 31449683892 31353787119 31424579134 31369350450",
    "65448746177 65283144945 65724916793 65315312562 65304274136",
    "135679030102 135795145770 135612939882 135429887029 135620156871",
    "282336883143 281318906428 281316496510 281223890307 280823483084",
    "582778018820 582959920564 582609200442 583766534358 583272193301",
]

dataset4 = [
    "4878570560 4852608033 4850688703 4848818201 4895562682",
    "9446761524 9434440939 9443852001 9426617550 9426677991",
    "19639014927 19494382436 19475693002 19497208608 19814683381",
    "39893235168 39886495856 39916621202 39860513654 39888492850",
    "81765489638 81813646250 81820961019 81920239576 81781277463",
    "167883249730 167931492190 167973698095 167826842082 167951457975",
]

# Function to calculate averages
def calculate_averages(dataset):
    averages = []
    for line in dataset:
        numbers = list(map(int, line.split()))
        averages.append(sum(numbers) / len(numbers))
    return averages

# Calculate averages for each dataset
averages1 = calculate_averages(dataset1)
averages2 = calculate_averages(dataset2)
averages3 = calculate_averages(dataset3)
averages4 = calculate_averages(dataset4)

# Array sizes and labels (you can modify these as needed)
array_sizes = [2**25, 2**26, 2**27, 2**28, 2**29, 2**30]
x_labels = ['2^25', '2^26', '2^27', '2^28', '2^29', '2^30']

# Create a bar graph comparing the averages for each dataset at each array size
fig, ax = plt.subplots(figsize=(10, 6))

# Bar width and positions for each dataset
bar_width = 0.15
index = np.arange(len(averages1))

# Plot bars for each dataset
bars1 = ax.bar(index - 1.5*bar_width, averages1, bar_width, label="Normal")
bars2 = ax.bar(index - 0.5*bar_width, averages2, bar_width, label="Static Malloc")
bars3 = ax.bar(index + 0.5*bar_width, averages3, bar_width, label="Tile")
bars4 = ax.bar(index + 1.5*bar_width, averages4, bar_width, label="Thread")

# Set titles and labels
ax.set_title("MergeSort Variations Comparision")
ax.set_xlabel("Array Size")
ax.set_ylabel("Average Ticks (100 bil)")
ax.set_xticks(index)
ax.set_xticklabels(x_labels)

# Add legend
ax.legend()

# Add grid lines for clarity
ax.grid(True)

# Show and save the plot
plt.tight_layout()
plt.savefig("dataset_comparison_bargraph.png")
# plt.show()
