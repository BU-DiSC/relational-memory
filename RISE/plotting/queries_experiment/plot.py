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

def read(filepath, skip):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        #bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired
        for bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(temp.strip(), dict()).setdefault(int(col_width.strip()), []).append(int(cycles))
    return res

def read_full(filepath, skip):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        #for bench, mem, temp, row_size, row_count, col_width, cycles in csv_reader:
        for bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(temp.strip(), dict()).setdefault(int(col_width.strip()), []).append(int(cycles))
    return res

def read_join(filepath, skip):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        for bench, mem, row_size, row_count, col_width, cycles in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(int(col_width.strip()), []).append(int(cycles))
    return res

def generate_plot(q, figsize, ncol=4, fontsize=False):
    plt.clf()
    if (fontsize):
        plt.rcParams.update({'font.size': 13.25})
    values = {}
    for i in range(4, 5, 1):
        res = read("PLT"+str(1)+"_result_q"+q+"_col.csv", 1)

        for memory in ('d', 'r', 'c'):
            if memory == 'r':
                tempList = ('c', 'h')
            elif memory == 'd' or 'c':
                tempList = ('-')
            for temperature in tempList:
                tmp = [e[1] for e in sorted(res[memory][temperature].items())]
                columns = sorted(res[memory][temperature].keys())
                res[memory][temperature] = dict()
                for index, col in enumerate(columns):
                    res[memory][temperature][col] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory][temperature].items())]
                std = [e[1]['std'] for e in sorted(res[memory][temperature].items())]
                res[memory][temperature] = {"avg" : avg, "std" : std}
        res = {'v'+str(i) : res}
        values = {**values, **res}

    labels = [2**i for i in range(5)]

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.15  # the width of the bars

    fig, ax = plt.subplots(figsize=figsize)
    #plt.rcParams.update({'font.size': 9.5})
    plt.rcParams['legend.handlelength'] = 1.35
    plt.rcParams['legend.handleheight'] = 1.125
    ax.plot([-0.5, len(columns)-0.5], [1.0,1.0], label="ROW", color=cmap(rescale(0)))
    ax.bar(x + -1.0*width*1.5,  np.divide(values['v4']['c']['-']['avg'], values['v4']['d']['-']['avg']), width*1.5, yerr=np.divide(values['v4']['c']['-']['std'],values['v4']['d']['-']['avg']), color=cmap(rescale(3)), edgecolor='black', label="COL")
    ax.bar(x +  0.0*width*1.5,  np.divide(values['v4']['r']['c']['avg'], values['v4']['d']['-']['avg']), width*1.5, yerr=np.divide(values['v4']['r']['c']['std'],values['v4']['d']['-']['avg']), color=cmap(rescale(1)), edgecolor='black', label="RME Cold")
    ax.bar(x +  1.0*width*1.5,  np.divide(values['v4']['r']['h']['avg'], values['v4']['d']['-']['avg']), width*1.5, yerr=np.divide(values['v4']['r']['h']['std'],values['v4']['d']['-']['avg']), color=cmap(rescale(2)), edgecolor='black', label="RME Hot")

    ax.set_ylabel('Norm. exec. time')
    ax.set_ylim([0.0, 1.1])
    ax.set_xlabel('Column width in Bytes')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), frameon=False, ncol=ncol, handletextpad=0.2, prop={'size':12} )

#    fig.tight_layout()

    plt.savefig("query"+str(q)+".pdf", bbox_inches='tight')
    plt.show()

def generate_full_plot(q, figsize, fontsize=False):
    plt.clf()
    if (fontsize):
        plt.rcParams.update({'font.size': 13.5})
    values = {}
    for i in range(4, 5, 1):
        res = read_full("PLT_result_q"+str(q)+"_col.csv", 1)
        for memory in ('r'):
            for temperature in ('c', 'h'):
                tmp = [e[1] for e in sorted(res[memory][temperature].items())]
                columns = sorted(res[memory][temperature].keys())
                res[memory][temperature] = dict()
                for index, col in enumerate(columns):
                    res[memory][temperature][col] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory][temperature].items())]
                std = [e[1]['std'] for e in sorted(res[memory][temperature].items())]
                res[memory][temperature] = {"avg" : avg, "std" : std}
        #
        try:
            for memory in ('d', 'c'):
                temperature = '-'
                tmp = [e[1] for e in sorted(res[memory][temperature].items())]
                columns = sorted(res[memory][temperature].keys())
                res[memory][temperature] = dict()
                for index, col in enumerate(columns):
                    res[memory][temperature][col] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory][temperature].items())]
                std = [e[1]['std'] for e in sorted(res[memory][temperature].items())]
                res[memory][temperature] = {"avg" : avg, "std" : std}
        except Exception as e:
            print("no 'dc' in PLT"+str(1 if (q < 2) else 2)+"_result_q"+str(q)+"_v"+str(i)+".csv")
            pass

        #
        res = {'v'+str(i) : res}
        values = {**values, **res}

    labels_str = [2**i for i in range(5)]

    x = np.arange(len(labels_str))  # the label locations
    width = 0.15  # the width of the bars


    fig, ax = plt.subplots(figsize=figsize)
    ax.plot([-0.5, len(labels_str)-0.5], [1.0,1.0], label="ROW", color=cmap(rescale(0)))
    ax.bar(x + -1.0*width*1.5,  np.divide(values['v4']['c']['-']['avg'], values['v4']['d']['-']['avg']), width*1.5, yerr=np.divide(values['v4']['c']['-']['std'],values['v4']['d']['-']['avg']), color=cmap(rescale(3)), edgecolor='black', label="COL")
    ax.bar(x +  0.0*width*1.5,  np.divide(values['v4']['r']['c']['avg'], values['v4']['d']['-']['avg']), width*1.5, yerr=np.divide(values['v4']['r']['c']['std'],values['v4']['d']['-']['avg']), color=cmap(rescale(1)), edgecolor='black', label="RME Cold")
    ax.bar(x +  1.0*width*1.5,  np.divide(values['v4']['r']['h']['avg'], values['v4']['d']['-']['avg']), width*1.5, yerr=np.divide(values['v4']['r']['h']['std'],values['v4']['d']['-']['avg']), color=cmap(rescale(2)), edgecolor='black', label="RME Hot")

    #fig.text(0.5, 0.3, "Column width in Bytes", ha="center", va="center")
    plt.xlabel('Column width in Bytes')

#    # axis 1
    ax.set_ylim([0.0, 1.1])
    ax.set_ylabel('Norm. exec. time')
    ax.set_xticks(x)
    ax.set_xlim([-0.5, 4.5])
    ax.set_xticklabels(labels_str)
    plt.rcParams['legend.handlelength'] = 1.43
    plt.rcParams['legend.handleheight'] = 1.125
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), frameon=False, ncol=4, prop={'size':12.5} )
#    # axis 2
#    ax2.yaxis.tick_right()
#    ax2.set_ylim([0.0, 1.75])
#    ax2.set_xticklabels(labels_str)
#
#    handles, labels = [(a + b) for a, b in zip(ax1.get_legend_handles_labels(), ax2.get_legend_handles_labels())]
#    labels = labels[:1]+[labels[5]]+labels[1:5]+labels[6:]
#    handles = handles[:1]+[handles[5]]+handles[1:5]+handles[6:]
#
#    ax2.legend(handles, labels, bbox_to_anchor=(1.125, -0.25), fancybox=False, shadow=False, ncol=4)

    plt.tight_layout()
#    plt.subplots_adjust(wspace=0.01, hspace=0)

    plt.savefig("query"+str(q)+".pdf")
    plt.show()

if (__name__ == '__main__'):
    generate_full_plot('1', (6.5, 3), True)
    generate_plot('2', (5.5, 2.2), 4, True)
    generate_plot('3', (5.5, 2.2), 4, True)
    generate_plot('4', (5.5, 2.2), 4, True)
