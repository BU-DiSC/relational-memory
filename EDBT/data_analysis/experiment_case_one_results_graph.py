# import matplotlib.pyplot as plt
# import numpy as np

#BAR GRAPH:

# r_c = [45244, 22642, 11347, 5724, 2901, 1505, 786, 415, 218, 132, 96, 72, 53]
# rh = [45100, 22561, 11290, 5675, 2840, 1436, 753, 376, 216, 109, 68, 48, 29]
# d = [65137, 31697, 16000, 8175, 4314, 2267, 1244, 1901, 996, 539, 298, 155, 109]
# c = [66229, 33106, 16551, 8237, 4132, 2121, 1148, 560, 294, 165, 82, 39, 21]

# x = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384])
# x_label = np.divide(np.arange(len(x)), 1)

# plt.xlabel("Row Size")
# plt.ylabel("Normalized Execution Time")
# plt.title("Effect of Type of Access on Row Size vs Runtime")

# bar_width = 0.2
# index = np.arange(len(x_label))

# plt.bar(index - bar_width, rh, width=bar_width, label='r h')
# plt.bar(index, r_c, width=bar_width, label='r c')
# plt.bar(index + bar_width, d, width=bar_width, label='d')
# plt.bar(index + (2 * bar_width), c, width=bar_width, label='c')

# plt.xticks(index, x)
# plt.legend()
# plt.show()


# NORMALIZED EXECUTION TIME:

import matplotlib.pyplot as plt
import numpy as np

r_c = np.array([45244, 22642, 11347, 5724, 2901, 1505, 786, 415, 218, 132, 96, 72, 53])
rh = np.array([45100, 22561, 11290, 5675, 2840, 1436, 753, 376, 216, 109, 68, 48, 29])
d = np.array([65137, 31697, 16000, 8175, 4314, 2267, 1244, 1901, 996, 539, 298, 155, 109])
c = np.array([66229, 33106, 16551, 8237, 4132, 2121, 1148, 560, 294, 165, 82, 39, 21])


normalized_time = d / d

x = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384])
x_label = np.divide(np.arange(len(x)), 1)

plt.xlabel("Row Size")
plt.ylabel("Normalized Execution Time")
plt.title("Effect of Type of Access on Row Size vs Runtime")

bar_width = 0.2
index = np.arange(len(x_label))

plt.bar(index - bar_width, rh / d, width=bar_width, label='r h')
plt.bar(index, r_c / d, width=bar_width, label='r c')
plt.bar(index + (1 * bar_width), c / d, width=bar_width, label='c')

plt.xticks(index, x)
plt.legend()
plt.ylim(0, 1.1) 
plt.axhline(y=1.0, color='red', linestyle='-')
plt.show()