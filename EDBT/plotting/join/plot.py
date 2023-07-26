#from tkinter import font
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import hmean
from matplotlib import gridspec
import csv

percentile = 95
revisions = 4
offsets = 60
cmap=plt.get_cmap('binary_r')
rescale = lambda x: (x - 0) / (3 - 0)
pale = lambda x: (x - 0) / (7 - 0)
period = 0.00000066666666666667 # mili second
pattern="//"

yoffsetbbox=1.25

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
        for bench, mem, row_size, row_count, col_width, access, hashing in csv_reader:
            if (not (int(row_size.strip()) in exceptions)):
                res.setdefault(mem.strip(),dict()).setdefault(int(row_size.strip()), []).append([period*int(access), period*int(hashing)])
    return res

def generate_plot(q, figsize, ncol=2, fontsize=False):
    plt.clf()
    if (fontsize):
        plt.rcParams.update({'font.size': 17})
    values = {}
    for i in range(4, 5, 1):
        res = read("PLT1_result_q"+q+".csv", 1)

        for memory in ('d', 'r', 'c'):
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
    width = 0.25  # the width of the bars

    fig, ax = plt.subplots(figsize=figsize)
    #plt.rcParams.update({'font.size': 9.5})
    ax.bar(x - 1.0*width,  np.divide(values['v4']['d']['avg'], values['v4']['d']['avg']), width, yerr=np.divide(values['v4']['d']['std'], values['v4']['d']['avg']), label="ROW", color=cmap(rescale(0)), edgecolor='black' )
    ax.bar(x + 0.0*width,  np.divide(values['v4']['c']['avg'], values['v4']['d']['avg']), width, yerr=np.divide(values['v4']['c']['std'], values['v4']['d']['avg']), label="COL", color=cmap(rescale(3)), edgecolor='black' )
    ax.bar(x + 1.0*width,  np.divide(values['v4']['r']['avg'], values['v4']['d']['avg']), width, yerr=np.divide(values['v4']['r']['std'], values['v4']['d']['avg']), label="RME", color=cmap(rescale(1)), edgecolor='black' )

    ax.set_ylabel('  Norm. exec. time  ')
    ax.set_ylim([0.0, 1.1])
    ax.set_xlabel('Column width in Bytes')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    #ax.legend(loc='upper center', bbox_to_anchor=(0.45, -0.35), fancybox=False, shadow=False, ncol=ncol)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, yoffsetbbox), frameon=False, ncol=ncol, prop={'size':12} )

#    fig.tight_layout()

    plt.savefig(q+".pdf", bbox_inches='tight')
    plt.show()

def generate_row_plot(q, exceptions, figsize, ncol, fontsize=False):
    plt.clf()
    if (fontsize):
        plt.rcParams.update({'font.size': 17})
    values = {}
    for i in range(4, 5, 1):
        res = read_row("PLT2_result_q"+q+".csv", 1, exceptions)
        for memory in ('d', 'r', 'c'):
                tmp = [e[1] for e in sorted(res[memory].items())]
                sizes = sorted(res[memory].keys())
                res[memory] = dict()
                for index, size in enumerate(sizes):
                    tempLine=list(zip(*tmp[index]))
                    res[memory][size] = {"data" : np.mean(tempLine[0]), "hash" : np.mean(tempLine[1]), "sum": np.mean(tempLine[0]) + np.mean(tempLine[1])}
                avg = [e[1]['data'] for e in sorted(res[memory].items())]
                std = [e[1]['hash'] for e in sorted(res[memory].items())]
                sum = [e[1]['sum' ] for e in sorted(res[memory].items())]
                res[memory] = {"data" : avg, "hash" : std, "sum" : sum}
        res = {'v'+str(i) : res}
        values = {**values, **res}

    labels = [2**(i+3) for i in range(1, len(values['v4']['r']['data'])+1) ]

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.25  # the width of the bars

    fig, ax = plt.subplots(figsize=figsize)
    plot_bars = []
    l1=ax.bar(x - 1.0*width, values['v4']['d']['hash'], width, label="ROW", color=cmap(rescale(0)), edgecolor=cmap(rescale(0)))
    l2=ax.bar(x - 1.0*width, values['v4']['d']['data'], width, bottom=values['v4']['d']['hash'], color='w', edgecolor=cmap(rescale(0)))
    ax.bar(x + 0.0*width, values['v4']['c']['hash'], width, label="COL", color=cmap(pale(6)), edgecolor='black' )
    ax.bar(x + 0.0*width, values['v4']['c']['data'], width, bottom=values['v4']['c']['hash'], color='w', edgecolor='black')
    ax.bar(x + 1.0*width, values['v4']['r']['hash'], width, label="RME", color=cmap(rescale(1)), edgecolor=cmap(rescale(0)))
    ax.bar(x + 1.0*width, values['v4']['r']['data'], width, bottom=values['v4']['r']['hash'], color='w', edgecolor=cmap(rescale(0)))

    plot_bars.append([l1, l2])

    ytick=[1,2,3]
    y=["  1", "  2", "  3"]
    ax.set_ylabel("Execution time (ms)")
    ax.set_yticks(ytick)
    ax.set_yticklabels(y)
    ax.set_xlabel('Row width in Bytes')
    ax.set_xlim([0-0.5, len(labels)-0.5])
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    plt.rcParams['legend.handlelength'] = 1.35
    plt.rcParams['legend.handleheight'] = 1.125
    legend1 = ax.legend(plot_bars[0], ["CPU", "Data"], loc='upper right', bbox_to_anchor=(1, yoffsetbbox-0.04), ncol=2, handletextpad=0.2,
        borderaxespad=0, frameon=False, prop={'size':12}, markerscale=0.7)
    legend1.legendHandles[0].set_edgecolor('black')
    legend1.legendHandles[1].set_color('w')
    legend1.legendHandles[0].set_color('black')
    legend1.legendHandles[1].set_edgecolor('black')
    legend2 = ax.legend(loc='upper left', bbox_to_anchor=(-0.2, yoffsetbbox), frameon=False,
        ncol=ncol, handletextpad=0.2, prop={'size':12}, markerscale=0.7)#labelspacing=-1.5)
    plt.gca().add_artist(legend2)
    plt.gca().add_artist(legend1)

#    fig.tight_layout()

    plt.savefig("join_row_size.pdf", bbox_inches='tight')
    plt.show()

if (__name__ == '__main__'):
    generate_plot('join', (5, 3), 3, True)
    generate_row_plot('join', [], (5, 3), 3)
