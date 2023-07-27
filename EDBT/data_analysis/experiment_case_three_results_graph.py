import matplotlib.pyplot as plt
import numpy as np

#BAR GRAPH:

# r_c = [78697,37030,18539,9299,4694,2387,1254,697,352,194,113,80,58]
# rh = [87095,38929,18462,9278,4643,2331,1197,618,313,191,123,60,59]
# d = [110381,59348,29083,16152,10139,3798,2835,3944,1904,1213,613,309,160]
# c = [66107,33141,16620,8236,4144,2147,1109,575,281,157,80,36,20]


# x = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384])
# x_label = np.divide(np.arange(len(x)), 1)

# plt.xlabel("Row Size")
# plt.ylabel("Runtime (ms)")
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

r_c = np.array([78697,37030,18539,9299,4694,2387,1254,697,352,194,113,80,58])
rh = np.array([87095,38929,18462,9278,4643,2331,1197,618,313,191,123,60,59])
d = np.array([110381,59348,29083,16152,10139,3798,2835,3944,1904,1213,613,309,160])
c = np.array([66107,33141,16620,8236,4144,2147,1109,575,281,157,80,36,20])


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