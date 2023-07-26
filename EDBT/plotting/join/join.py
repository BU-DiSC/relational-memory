import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import hmean
from matplotlib import gridspec
import csv

percentile = 95
revisions = 4
offsets = 60
colors = ["red", "green", "blue", "purple"]
period = 0.00000066666666666667 # mili second

def read(filepath, skip):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        for bench, mem, row_size, row_count, col_width, cycles in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(int(col_width.strip()), []).append(int(cycles))
    return res

def read_row(filepath, skip, exceptions):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        for bench, mem, row_size, row_count, col_width, cycles in csv_reader:
            if (not (int(row_size.strip()) in exceptions)):
                res.setdefault(mem.strip(),dict()).setdefault(int(row_size.strip()), []).append(period * int(cycles))
    return res

def generate_plot(q, figsize, ncol=2, fontsize=False):
    plt.clf()
    if (fontsize):
        plt.rcParams.update({'font.size': 11.25})
    values = {}
    for i in range(4, 5, 1):
        res = read("PLT"+str(2)+"_result_q"+q+".csv", 1)

        for memory in ('d', 'r'):
                tmp = [e[1] for e in sorted(res[memory].items())]
                columns = sorted(res[memory].keys())
                res[memory] = dict()
                for index, col in enumerate(columns):
                    res[memory][col] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory].items())]
                std = [e[1]['std'] for e in sorted(res[memory].items())]
                res[memory] = {"avg" : avg, "std" : std}
        res = {'v'+str(i) : res}
        values = {**values, **res}

    labels = [2**i for i in range(5)]

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.3  # the width of the bars

    fig, ax = plt.subplots(figsize=figsize)
    plt.rcParams.update({'font.size': 9.5})
    ax.bar(x - 0.5*width,  np.divide(values['v4']['d']['avg'], values['v4']['d']['avg']), width, yerr=np.divide(values['v4']['d']['std'], values['v4']['r']['avg']), label="Direct Row-wise", color="tab:red")
    ax.bar(x + 0.5*width,  np.divide(values['v4']['r']['avg'], values['v4']['d']['avg']), width, yerr=np.divide(values['v4']['r']['std'], values['v4']['r']['avg']), label="RME", color="tab:blue")

    ax.set_ylabel('Normalized execution time')
    ax.set_ylim([0.0, 1.1])
    ax.set_xlabel('Column width in Bytes')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend(loc='upper center', bbox_to_anchor=(0.45, -0.25), fancybox=False, shadow=False, ncol=ncol)

    fig.tight_layout()

    plt.savefig(q+".pdf", bbox_inches='tight')
    plt.show()

def generate_row_plot(q, exceptions, figsize, ncol, yoffsetbbox=-0.5):
    plt.clf()
    values = {}
    for i in range(4, 5, 1):
        res = read_row("PLT"+str(2)+"_result_q"+q+"_row_size.csv", 1, exceptions)
        for memory in ('d', 'r'):
                tmp = [e[1] for e in sorted(res[memory].items())]
                sizes = sorted(res[memory].keys())
                res[memory] = dict()
                for index, size in enumerate(sizes):
                    res[memory][size] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                #print("------------------------------------------------------------")
                #print(memory, temperature)
                #print(sorted(res[memory][temperature].items()))
                avg = [e[1]['avg'] for e in sorted(res[memory].items())]
                std = [e[1]['std'] for e in sorted(res[memory].items())]
                res[memory] = {"avg" : avg, "std" : std}
                #print("->")
                #print(res[memory][temperature])
                #print("------------------------------------------------------------")
        #for memory in ('d', 'r'):
        #    if memory == 'r':
        #        tempList = ('c', 'h')
        #    elif memory == 'd':
        #        tempList = ('-')
        #    for temperature in tempList:
        #        print(memory, temperature)
        #        print(res[memory][temperature])
        res = {'v'+str(i) : res}
        values = {**values, **res}

    #print(values)

    #labels = [i*4 if (i%4 == 0) else "" for i in range(1+len(exceptions), 33)]
    labels = [2**(i+3) for i in range(1, len(values['v4']['r']['avg'])+1) ]

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.3  # the width of the bars
    print(labels,x)
    print( values['v4']['r']['avg'] )

    fig, ax = plt.subplots(figsize=figsize)
    ax.bar(x - 0.5*width,  np.divide(values['v4']['d']['avg'], 1), width, yerr=np.divide(values['v4']['d']['std'], values['v4']['d']['avg']), label="Direct Row-wise", color="tab:red")
    ax.bar(x + 0.5*width,  np.divide(values['v4']['r']['avg'], 1), width, yerr=np.divide(values['v4']['r']['std'], values['v4']['d']['avg']), label="RME cold", color="tab:blue")

    ax.set_ylabel('Execution time (ms)')
#    ax.set_ylim([0.0, 1.1])
    ax.set_xlabel('Row width in Bytes')
    ax.set_xlim([0-0.5, len(labels)-0.5])
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend(loc='lower center', bbox_to_anchor=(0.425, yoffsetbbox), fancybox=False, shadow=False, ncol=ncol)

    fig.tight_layout()

    plt.savefig("join_row_size.pdf", bbox_inches='tight')
    plt.show()

if (__name__ == '__main__'):
    generate_plot('join', (5, 3), 3, True)
    generate_row_plot('join', [48, 80, 96, 112, 144, 160, 176, 192, 208, 224, 240], (5, 3), 3)
