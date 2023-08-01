import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import hmean
from matplotlib import gridspec
import csv
import math

percentile = 95
revisions = 4
offsets = 60
#period = 0.00066666666666667 # micro second
period = 0.00000066666666666667 # mili second

cmap=plt.get_cmap('binary_r')
rescale = lambda x: (x - 0) / (3 - 0)
colors = [cmap(rescale(0)), cmap(rescale(1)), cmap(rescale(2)), cmap(rescale(3))]

CONFIG = 429.605011
RELCACHE = 2*1024*1024
ROW_SIZE = 200
TARGET = 2

def read(filepath, skip, query):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)

        for bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(int(row_count.strip()), []).append(period*int(cycles))
        #if query==1 :
        #    for bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired in csv_reader:
        #        res.setdefault(mem.strip(),dict()).setdefault(int(row_count.strip()), []).append(period*int(cycles))
        #elif query==6 :
        #    for bench, mem, temp, row_size, row_count, col_width, cycles, temp in csv_reader:
        #        res.setdefault(mem.strip(),dict()).setdefault(int(row_count.strip()), []).append(period*int(cycles))

    return res

def generate_plot(query, figsize, ncol=2, fontsize=False):
    plt.clf()
    plt.rcParams.update({'font.size': 17})
    values = {}
    if query==1:
        column_size = 47
    elif query==6:
        column_size = 37
    for i in range(4, 5, 1):
        res = read("PLT_result_tpch_q"+str(query)+".csv", 1, query)
        for memory in ('d', 'r', 'c'):
                tmp = [e[1] for e in sorted(res[memory].items())]
                row_counts = sorted(res[memory].keys())
                res[memory] = dict()
                for index, rc in enumerate(row_counts):
                    if memory == 'r':
                        dbsize = column_size * int(rc)
                        partition = math.ceil( dbsize/RELCACHE )
                        new = [ period*partition*CONFIG + i for i in tmp[index] ]
                        res[memory][rc] = {"avg" : np.mean(new), "std" : np.std(new)}
                    else:
                        res[memory][rc] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory].items())]
                std = [e[1]['std'] for e in sorted(res[memory].items())]
                res[memory] = {"avg" : avg, "std" : std}
        res = {'v'+str(i) : res}
        values = {**values, **res}

    dataSize = [ROW_SIZE*i for i in row_counts]
    colSize  = [column_size*i for i in row_counts]
    collabel = []
    for i in range(0, len(colSize)):
        collabel.append( str(math.ceil(colSize[i]/(1024*1024))) )
        #if colSize[i] < 1024 :
        #    collabel.append( str(colSize[i])+"B" )
        #elif colSize[i] < 1024*1024 :
        #    collabel.append( str(int(colSize[i]/1024))+"KB" )
        #elif colSize[i] < 1024*1024*1024 :
        #    collabel.append( str(int(colSize[i]/(1024*1024)))+"MB" )
    labels = []
    for i in range(0, len(dataSize)):
        labels.append( str(math.ceil(dataSize[i]/(1024*1024)))+"\n"+"("+collabel[i]+")" )
        #if dataSize[i] < 1024 :
        #    labels.append( str(dataSize[i])+"B\n"+"("+collabel[i]+")" )
        #elif dataSize[i] < 1024*1024 :
        #    labels.append( str(int(dataSize[i]/1024))+"KB\n"+"("+collabel[i]+")" )
        #elif dataSize[i] < 1024*1024*1024 :
        #    labels.append( str(int(dataSize[i]/(1024*1024)))+"MB\n"+"("+collabel[i]+")" )
        #elif dataSize[i] < 1024*1024*1024*1024 :
        #    labels.append( str(int(dataSize[i]/(1024*1024*1024)))+"GB\n"+"("+collabel[i]+")" )
    print(labels)

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.3  # the width of the bars

    print(values['v4']['d']['avg'])
    print(values['v4']['r']['avg'])
    print(values['v4']['c']['avg'])
    min = np.min(values['v4']['r']['avg'])
    max = np.max(values['v4']['d']['avg'])

    fig, ax = plt.subplots(figsize=figsize)
    ax.plot(x, values['v4']['d']['avg'], label="ROW", color=colors[0], markersize=11.5, marker='x')
    ax.plot(x, values['v4']['c']['avg'], label="COL", color=colors[2], markersize=11.5, marker='s', fillstyle='none')
    ax.plot(x, values['v4']['r']['avg'], label="RME", color=colors[1], markersize=11.5, marker='^', fillstyle='none')

    ax.set_yscale('log')
    ax.set_ylabel('Execution time (ms)')
    ax.set_xlabel('Data size (Taget column size (MB)')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    #ax.set_xticklabels(labels, fontsize=10)
    #plt.xticks(rotation=35)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), frameon=False, ncol=ncol)

    #fig.tight_layout()

    plt.savefig("framing_q"+str(query)+".pdf", bbox_inches='tight')
    plt.show()

if (__name__ == '__main__'):
    generate_plot(1, (5, 2.5), 3)
    generate_plot(6, (5, 2.5), 3)
