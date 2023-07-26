import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import hmean
from matplotlib import gridspec
import csv

percentile = 95
revisions = 4
offsets = 60
colors = ["red", "green", "blue", "purple"]
#period = 0.00066666666666667 # micro second
period = 0.00000066666666666667 # mili second

def read(filepath, skip):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
#            benchmark,  memory, row_size, row_count, time
        for bench, mem, row_size, row_count, cycles in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(int(row_count.strip()), []).append(period*int(cycles))
    return res

def generate_plot(row_size, column_size, figsize, ncol=2, fontsize=False):
    plt.clf()
    if (fontsize):
        plt.rcParams.update({'font.size': 11.25})
    values = {}
    for i in range(4, 5, 1):
        res = read("results_framing.csv", 1)
        for memory in ('d', 'r', 'c'):
                tmp = [e[1] for e in sorted(res[memory].items())]
                row_counts = sorted(res[memory].keys())
                res[memory] = dict()
                for index, rc in enumerate(row_counts):
                    res[memory][rc] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory].items())]
                std = [e[1]['std'] for e in sorted(res[memory].items())]
                res[memory] = {"avg" : avg, "std" : std}
        res = {'v'+str(i) : res}
        values = {**values, **res}

    #print ( values )
    dataSize = [row_size*i for i in row_counts]
    colSize  = [column_size*i for i in row_counts]
    collabel = []
    for i in range(0, len(colSize)):
        if colSize[i] < 1024 :
            collabel.append( str(colSize[i])+"B" )
        elif colSize[i] < 1024*1024 :
            collabel.append( str(int(colSize[i]/1024))+"KB" )
        elif colSize[i] < 1024*1024*1024 :
            collabel.append( str(int(colSize[i]/(1024*1024)))+"MB" )
    labels = []
    for i in range(0, len(dataSize)):
        if dataSize[i] < 1024 :
            labels.append( str(dataSize[i])+"B\n"+"("+collabel[i]+")" )
        elif dataSize[i] < 1024*1024 :
            labels.append( str(int(dataSize[i]/1024))+"KB\n"+"("+collabel[i]+")" )
        elif dataSize[i] < 1024*1024*1024 :
            labels.append( str(int(dataSize[i]/(1024*1024)))+"MB\n"+"("+collabel[i]+")" )
        elif dataSize[i] < 1024*1024*1024*1024 :
            labels.append( str(int(dataSize[i]/(1024*1024*1024)))+"GB\n"+"("+collabel[i]+")" )
    print(dataSize)
    print(labels)

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.3  # the width of the bars

    fig, ax = plt.subplots(figsize=figsize)
    plt.rcParams.update({'font.size': 9.5})
#    plt.errorbar(x, values['v4']['d']['avg'], yerr=values['v4']['d']['std'], label="Direct Access", color="tab:red")
#    plt.errorbar(x, values['v4']['r']['avg'], yerr=values['v4']['r']['std'], label="RME", color="tab:blue")
#    plt.errorbar(x, values['v4']['c']['avg'], yerr=values['v4']['c']['std'], label="Columnar Access", color="tab:gray")
    ax.plot(x, values['v4']['d']['avg'], label="Direct Access", color="tab:red", linewidth=2)
    ax.plot(x, values['v4']['r']['avg'], label="RME", color="tab:blue", linewidth=2)
#    ax.plot(x, values['v4']['c']['avg'], label="Columnar Access", color="tab:gray")

#    ax.set_yscale('log')
    ax.set_ylabel('Execution time (ms)', fontsize=9)
#    ax.set_ylim([0.0, 1.1])
    ax.set_xlabel('Data size (Taget column size)', fontsize=9)
    ax.set_xticks(x)
    ax.set_xticklabels(labels, fontsize=9)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.27), fancybox=False, shadow=False, ncol=ncol)

    fig.tight_layout()

    plt.savefig("framing.pdf", bbox_inches='tight')
    plt.show()

if (__name__ == '__main__'):
    generate_plot(256, 16, (5, 2.5), 3, True)
