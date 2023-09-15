import csv
import math

from matplotlib import pyplot as plt
import numpy as np

percentile = 95
revisions = 4
offsets = 60
period = 0.00000066666666666667  # millisecond

cmap = plt.get_cmap('binary_r')
rescale = lambda x: (x - 0) / (3 - 0)
colors = [cmap(rescale(0)), cmap(rescale(1)), cmap(rescale(2)), cmap(rescale(3))]

CONFIG = 429.605011
RELCACHE = 2 * 1024 * 1024
ROW_SIZE = 200
TARGET = 2


def read(filepath, skip):
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(skip):
            next(csv_reader)

        for bench, mem, temp, tmp, row_size, row_count, col_width, cycles in csv_reader:
            res.setdefault(mem.strip(), dict()).setdefault(int(row_count.strip()), []).append(period * int(cycles))

    return res


def generate_plot(figsize, ncol=2):
    plt.rcParams.update({'font.size': 17})
    values = {}
    column_size = 32

    for i in range(4, 5, 1):
        res = read('result_scale.csv', 1)
        for memory in ('d', 'r', 'c'):
            tmp = [e[1] for e in sorted(res[memory].items())]
            row_counts = sorted(res[memory].keys())
            res[memory] = dict()
            for index, rc in enumerate(row_counts):
                if memory == 'r':
                    dbsize = column_size * int(rc)
                    partition = math.ceil(dbsize / RELCACHE)
                    new = [period * partition * CONFIG + i for i in tmp[index]]
                    res[memory][rc] = {'avg': np.mean(new), 'std': np.std(new)}
                else:
                    res[memory][rc] = {'avg': np.mean(tmp[index]), 'std': np.std(tmp[index])}
            avg = [e[1]['avg'] for e in sorted(res[memory].items())]
            std = [e[1]['std'] for e in sorted(res[memory].items())]
            res[memory] = {'avg': avg, 'std': std}
        res = {'v' + str(i): res}
        values = {**values, **res}

    dataSize = [ROW_SIZE * i for i in row_counts]
    colSize = [column_size * i for i in row_counts]
    collabel = []
    for i in range(0, len(colSize)):
        collabel.append(str(math.ceil(colSize[i] / (1024 * 1024))))
    labels = []
    for i in range(0, len(dataSize)):
        labels.append(str(math.ceil(dataSize[i] / (1024 * 1024))) + '\n' + '(' + collabel[i] + ')')

    x = np.divide(np.arange(len(labels)), 1)  # the label locations

    fig, ax = plt.subplots(figsize=figsize)
    ax.plot(x, values['v4']['d']['avg'], label='ROW', color=colors[0], markersize=11.5, marker='x')
    ax.plot(x, values['v4']['c']['avg'], label='COL', color=colors[2], markersize=11.5, marker='s', fillstyle='none')
    ax.plot(x, values['v4']['r']['avg'], label='RME', color=colors[1], markersize=11.5, marker='^', fillstyle='none')

    ax.set_yscale('log')
    ax.set_ylabel('Execution time (ms)')
    ax.set_xlabel('Data size (Taget column size (MB)')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), frameon=False, ncol=ncol)

    plt.show()


def plot(refresh=False):
    generate_plot((5, 2.5), 3)


if __name__ == '__main__':
    plot()
