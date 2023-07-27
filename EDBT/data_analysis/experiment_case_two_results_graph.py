import matplotlib.pyplot as plt
import numpy as np

#BAR GRAPH:

r_c = [45256,22635,11352,5717,2913,1509,798,406,225,128,98,65,53]
rh = [47076,22568,11284,5646,2849,1436,727,365,199,112,68,45,23]
d = [63095,31668,15978,8187,4378,2427,1241,1948,998,546,279,158,9]
c = [66208,33057,16550,8243,4121,2175,1120,588,291,164,98,59,47]

x = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384])
x_label = np.divide(np.arange(len(x)), 1)

plt.xlabel("Row Size")
plt.ylabel("Runtime (ms)")
plt.title("Effect of Type of Access on Row Size vs Runtime")

bar_width = 0.2
index = np.arange(len(x_label))

plt.bar(index - bar_width, rh, width=bar_width, label='r h')
plt.bar(index, r_c, width=bar_width, label='r c')
plt.bar(index + bar_width, d, width=bar_width, label='d')
plt.bar(index + (2 * bar_width), c, width=bar_width, label='c')

plt.xticks(index, x)
plt.legend()
plt.show()


# NORMALIZED EXECUTION TIME:

# r_c = np.array([45256,22635,11352,5717,2913,1509,798,406,225,128,98,65,53])
# rh = np.array([47076,22568,11284,5646,2849,1436,727,365,199,112,68,45,23])
# d = np.array([63095,31668,15978,8187,4378,2427,1241,1948,998,546,279,158,99]) 
# c = np.array([66208,33057,16550,8243,4121,2175,1120,588,291,164,98,59,47])


# normalized_time = d / d

# x = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384])
# x_label = np.divide(np.arange(len(x)), 1)

# plt.xlabel("Row Size")
# plt.ylabel("Normalized Execution Time")
# plt.title("Effect of Type of Access on Row Size vs Runtime")

# bar_width = 0.2
# index = np.arange(len(x_label))

# plt.bar(index - bar_width, rh / d, width=bar_width, label='r h')
# plt.bar(index, r_c / d, width=bar_width, label='r c')
# plt.bar(index + (1 * bar_width), c / d, width=bar_width, label='c')

# plt.xticks(index, x)
# plt.legend()
# plt.ylim(0, 1.1)  
# plt.axhline(y=1.0, color='red', linestyle='-')

# plt.show()