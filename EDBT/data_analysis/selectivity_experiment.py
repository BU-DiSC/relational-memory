import matplotlib.pyplot as plt
import numpy as np

#BAR GRAPH:

# x = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288])
# x_label = np.divide(np.arange(len(x)), 1)

# plt.xlabel("Row Size")
# plt.ylabel("Cache miss ratio(refills/references)")
# plt.title("l2 refills/references")

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

#Time(ms)
# c = np.array([143660.6, 143904.6, 142610.1, 143770.5, 143161.7, 142787.8, 142591.9, 143129.8, 142477.8, 142279.6, 143125.8])
# d = np.array([127410.1, 129360.2, 130001.6, 133967.2, 141423.7, 157826.3, 187403.4, 249346.9, 378628.7, 620001.2, 1121537.2])
# r_c = np.array([125920.7, 125080.7, 125703.5, 125348.3, 125741.9, 125497.2, 125451.6, 125293.8, 125751.5, 125204.3, 125743.1])
# rh = np.array([125552.4, 124786.5, 124853.5, 125314.2, 126049.2, 124684.1, 125111.5, 125069.4, 124651.9, 125388.6, 124841.4])

#Time with min and max(ms)
# c = np.array([142824.5, 143458.4, 142547.7, 142686.3, 142923.0, 143144.5, 142915.9, 145400.3, 143242.7, 143374.6, 142096.5])
# d = np.array([127551.1, 127709.3, 130213.6, 134431.1, 141735.8, 157190.2, 189565.2, 250426.9, 374949.4, 626764.4, 1103845.4])
# r_c = np.array([125236.9, 125137.7, 125063.3, 125746.6, 125797.1, 125361.5, 125802.5, 125941.7, 124591.8, 125693.0, 125344.8])
# rh = np.array([125239.8, 126324.4, 125235.2, 124690.0, 124474.5, 125374.2, 124758.5, 124800.0, 124861.7, 125203.4, 125032.5])

#Time with min and max(ms) and .0015%% selectivity
# c = np.array([142600.1, 143231.5, 143110.5, 142775.8, 143459.4, 143750.5, 143482.5, 142916.6, 143523.9, 142086.6, 142032.2])
# d = np.array([127296.9, 129400.1, 130178.1, 133875.5, 141344.8, 157429.9, 188807.6, 252445.8, 381593.6, 620897.8, 1111234.3])
# r_c = np.array([125357.5, 125055.6, 125261.6, 125522.3, 125516.0, 125116.9, 125612.4, 125912.0, 125337.0, 125172.8, 125527.6])
# rh = np.array([125000.4, 124539.8, 125141.1, 124760.5, 124976.3, 124671.6, 124622.5, 124804.8, 124771.5, 125000.0, 125090.7])

#Time with min and max(ms) and 10% selectivity
c = np.array([142721.2, 143609.4, 142393.2, 143783.9, 143624.2, 142942.0, 143346.4, 143353.8, 144037.8, 142268.4, 142564.9])
d = np.array([127511.5, 127793.4, 130166.9, 133875.4, 141832.4, 158496.6, 187934.8, 251587.0, 371215.3, 647451.9, 1124552.0])
r_c = np.array([127260.9, 127167.8, 125575.9, 125231.3, 125006.6, 125597.5, 126875.8, 127158.8, 125957.9, 126005.6, 127748.3])
rh = np.array([124549.5, 124375.1, 125335.8, 124960.7, 124888.3, 124709.6, 125204.2, 124526.5, 125326.9, 125032.5, 125129.8])

#Time with min and max(ms) and 75% selectivity
c = np.array([142837.5, 143153.6, 142331.8, 144381.8, 143867.9, 142596.3, 142568.6, 142525.3, 143414.2, 143131.1, 142585.3])
d = np.array([127438.6, 128214.6, 130039.5, 133806.6, 141268.2, 207938.7, 187909.7, 249988.8, 375883.8, 618682.9, 1104502.3])
r_c = np.array([125346.2, 125014.4, 126316.4, 125569.7, 126024.8, 125805.3, 125590.7, 125134.7, 125550.4, 124966.6, 124872.7])
rh = np.array([125002.1, 125567.4, 125025.8, 124910.6, 124526.6, 129727.4, 125051.0, 124536.5, 124804.9, 125191.6, 124777.5])


normalized_time = d / d

x = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096])
x_label = np.divide(np.arange(len(x)), 1)

plt.xlabel("Row Size")
plt.ylabel("Normalized Execution Time")
plt.title("Selectivity Experiment - Row Size vs Execution Time")

bar_width = 0.2
index = np.arange(len(x_label))

plt.bar(index - bar_width, rh / d, width=bar_width, label='r h')
plt.bar(index, r_c / d, width=bar_width, label='r c')
plt.bar(index + (1 * bar_width), c / d, width=bar_width, label='c')

plt.xticks(index, x)
plt.legend()
plt.ylim(0, 1.2)  
plt.axhline(y=1.0, color='red', linestyle='-')
plt.text(index[-1] + bar_width + 0.8, 1.0, 'row', va='center', ha='left', color='black')

plt.show()