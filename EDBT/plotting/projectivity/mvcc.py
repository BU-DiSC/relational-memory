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
        for bench, mem, temp, mvcc, proj, row_size, row_count, col_width, cycles in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(int(mvcc.strip()), dict()).setdefault(int(proj.strip()), []).append(int(cycles))
            #res.setdefault(mem.strip(),dict()).setdefault(temp.strip(), dict()).setdefault(int(col_width.strip(    )), []).append(int(cycles))
    return res

def generate_full_plot(figsize, m, filename):
    plt.clf()
    plt.rcParams.update({'font.size': 14})
    values = {}
    for i in range(4, 5, 1):
        res = read("PLT_result_mvcc.csv", 1)
        for memory in ('r', 'd', 'c'):
            for mvcc in range(2):
#                print(mvcc)
                tmp = [e[1] for e in sorted(res[memory][mvcc].items())]
                columns = sorted(res[memory][mvcc].keys())
                res[memory][mvcc] = dict()
                for index, col in enumerate(columns):
                    res[memory][mvcc][col] = {"avg" : np.mean(tmp[index]), "std" : np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory][mvcc].items())]
                std = [e[1]['std'] for e in sorted(res[memory][mvcc].items())]
                res[memory][mvcc] = {"avg" : avg, "std" : std}
        #
        res = {'v'+str(i) : res}
        values = {**values, **res}

    labels_str = [i+1 for i in range(len(values['v4']['r'][0]['avg']))]

    x = np.arange(len(labels_str))  # the label locations
    width = 0.20  # the width of the bars

    #print(values['v4']['d'][m]['avg'])
    #print(values['v4']['r'][m]['avg'])
    #print(values['v4']['c'][m]['avg'])
    #print(values['v4']['d'][m]['std'])
    #print(values['v4']['r'][m]['std'])
    #print(values['v4']['c'][m]['std'])

    fig, ax = plt.subplots(figsize=figsize)
    ax.plot([-0.5, len(labels_str)-0.5], [1.0,1.0], label="ROW", color=cmap(rescale(0)))
    ax.bar(x + -0.5*width*1.5,  np.divide(values['v4']['c'][m]['avg'], values['v4']['d'][m]['avg']), width*1.5, yerr=np.divide(values['v4']['c'][m]['std'],values['v4']['d'][m]['avg']), color=cmap(rescale(3)), label="COL", edgecolor='black')
    ax.bar(x +  0.5*width*1.5,  np.divide(values['v4']['r'][m]['avg'], values['v4']['d'][m]['avg']), width*1.5, yerr=np.divide(values['v4']['r'][m]['std'],values['v4']['d'][m]['avg']), color=cmap(rescale(1)), label="RME Cold")

    plt.xlabel('Projectivity (Number of target columns)')

    ax.set_ylim([0.0, 1.3])
    ax.set_ylabel('Norm. exec time')
    ax.set_xticks(x)
    ax.set_xlim([-0.5, len(labels_str)-0.5])
    ax.set_xticklabels(labels_str)
    ax.legend(loc='upper center', bbox_to_anchor=(0.45, -0.35), fancybox=False, shadow=False, ncol=4)

    plt.tight_layout()

    plt.savefig(filename)
    plt.show()

if (__name__ == '__main__'):
    generate_full_plot((6.5, 3), 0, "projectvity.pdf")
    generate_full_plot((6.5, 3), 1, "projectvity_mvcc.pdf")
