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
periodmus = 0.00066666666666667 # micro second
periodms  = 0.00000066666666666667 # micro second

def read(filepath, skip, exceptions, scale):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        for bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired in csv_reader:
            if (not (int(row_size.strip()) in exceptions)):
                if (scale=='mus'):
                    res.setdefault(mem.strip(),dict()).setdefault(temp.strip(), dict()).setdefault(int(row_size.strip()), []).append(periodmus * int(cycles))
                elif (scale=='ms'):
                    res.setdefault(mem.strip(),dict()).setdefault(temp.strip(), dict()).setdefault(int(row_size.strip()), []).append(periodms * int(cycles))
    return res

def generate_plot(q, exceptions, figsize, ncol, yoffsetbbox=1):
    plt.clf()
    values = {}
    plt.rcParams.update({'font.size': 13.5})
    for i in range(4, 5, 1):
        if q==4:
          res = read("PLT2_result_q"+str(q)+"_col.csv", 1, exceptions, 'ms')
        else :
          res = read("PLT2_result_q"+str(q)+"_col.csv", 1, exceptions, 'mus')
        for memory in ('d', 'r', 'c'):
            if memory == 'r':
                tempList = ('c', 'h')
            elif memory == 'd' or 'c':
                tempList = ('-')
            for temperature in tempList:
                tmp = [e[1] for e in sorted(res[memory][temperature].items())]
                sizes = sorted(res[memory][temperature].keys())
                res[memory][temperature] = dict()
                for index, size in enumerate(sizes):
                    res[memory][temperature][size] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory][temperature].items())]
                std = [e[1]['std'] for e in sorted(res[memory][temperature].items())]
                res[memory][temperature] = {"avg" : avg, "std" : std}
        res = {'v'+str(i) : res}
        values = {**values, **res}

    #print(values)

    #labels = [i*4 if (i%4 == 0) else "" for i in range(1+len(exceptions), 33)]
    labels = [2**(i+3) for i in range(1, len(values['v4']['r']['h']['avg'])+1) ]

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.20  # the width of the bars

    fig, ax = plt.subplots(figsize=figsize)
    #plt.rcParams.update({'font.size': 9.5})
    ax.bar(x - 1.5*width,  np.divide(values['v4']['d']['-']['avg'], 1), width, yerr=values['v4']['r']['c']['std'], color=cmap(rescale(0)), edgecolor='black', label="ROW" )
    ax.bar(x - 0.5*width,  np.divide(values['v4']['c']['-']['avg'], 1), width, yerr=values['v4']['c']['-']['std'], color=cmap(rescale(3)), edgecolor='black', label="COL" )
    ax.bar(x + 0.5*width,  np.divide(values['v4']['r']['c']['avg'], 1), width, yerr=values['v4']['r']['c']['std'], color=cmap(rescale(1)), edgecolor='black', label="RME cold" )
    ax.bar(x + 1.5*width,  np.divide(values['v4']['r']['h']['avg'], 1), width, yerr=values['v4']['r']['h']['std'], color=cmap(rescale(2)), edgecolor='black', label="RME Hot" )

    print(values['v4']['d']['-']['avg']) 
    print(values['v4']['r']['c']['avg']) 
    if q==4 :
        ax.set_ylabel('Execution time (ms)')
    else :
        ax.set_ylabel('Execution time (\u03BCs)')
#    ax.set_ylim([0.0, 1.1])
    ax.set_xlabel('Row width in Bytes')
    ax.set_xlim([0-0.5, len(labels)-0.5])
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    plt.rcParams['legend.handlelength'] = 1.43
    plt.rcParams['legend.handleheight'] = 1.125
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, yoffsetbbox+0.25), frameon=False, ncol=ncol, handletextpad=0.2, prop={'size':12} )

#    fig.tight_layout()

    plt.savefig("query"+str(q)+"_row_size.pdf", bbox_inches='tight')
    plt.show()

if (__name__ == '__main__'):
    generate_plot(2, [], (5.5, 2.2), 4)
    generate_plot(3, [], (5.5, 2.2), 4)
    generate_plot(4, [], (5.5, 2.2), 4)
