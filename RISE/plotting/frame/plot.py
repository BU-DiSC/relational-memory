import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import hmean
from matplotlib import gridspec
import csv

percentile = 95
revisions = 4
offsets = 60
colors = ["red", "green", "blue", "purple"]
period = 0.00066666666666667

def read(filepath, skip):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
#            benchmark,  memory, row_size, row_count, time
        for bench, mem, row_size, row_count, cycles in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(int(row_count.strip()), []).append(int(cycles))
    return res

def generate_plot(row_size, column_size, figsize, ncol=2, fontsize=False):
    plt.clf()
#    if (fontsize):
    plt.rcParams.update({'font.size': 8})
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
    ax.plot([-0.5, len(labels)-0.5], [1.0,1.0], label="Direct Row-wise", color="red")
    ax.bar(x,  np.divide(values['v4']['r']['avg'], values['v4']['d']['avg']), width, yerr=np.divide(values['v4']['r']['std'], values['v4']['d']['avg']), label="RME")
#    ax.bar(x + 0.5*width,  np.divide(values['v4']['c']['avg'], values['v4']['d']['avg']), width, yerr=np.divide(values['v4']['c']['std'], values['v4']['r']['avg']), label="Columnar Access")

    y=[0, 0.25, 0.5, 0.75, 1]
    ax.set_ylabel('Normalized execution time')
    ax.set_ylim([0.0, 1.1])
    ax.set_yticks(y)
    ax.set_yticklabels(y)
    ax.set_xlabel('Data size (Taget column size)')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.21), fancybox=False, shadow=False, ncol=ncol)

    fig.tight_layout()

    plt.savefig("framing.pdf", bbox_inches='tight')
    plt.show()

if (__name__ == '__main__'):
    generate_plot(256, 16, (4.5, 2.3), 3, True)
