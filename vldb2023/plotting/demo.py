import csv

from matplotlib import pyplot as plt
import numpy as np

percentile = 95
revisions = 4
offsets = 60
cmap = plt.get_cmap('binary_r')
rescale = lambda x: x / 3


def read_full(filepath, skip):
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(skip):
            next(csv_reader)
        for bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired in csv_reader:
            res.setdefault(mem.strip(), dict()).setdefault(temp.strip(), dict()).setdefault(int(col_width.strip()),
                                                                                            []).append(int(cycles))
    return res


def generate_full_plot(figsize, fontsize=False):
    if fontsize:
        plt.rcParams.update({'font.size': 13.5})
    values = {}
    for i in range(4, 5, 1):
        res = read_full('result_demo.csv', 1)
        for memory in ('r',):
            for temperature in ('c', 'h'):
                tmp = [e[1] for e in sorted(res[memory][temperature].items())]
                columns = sorted(res[memory][temperature].keys())
                res[memory][temperature] = dict()
                for index, col in enumerate(columns):
                    res[memory][temperature][col] = {'avg': np.mean(tmp[index]), 'std': np.std(tmp[index])}
                avg = [e[1]['avg'] for e in sorted(res[memory][temperature].items())]
                std = [e[1]['std'] for e in sorted(res[memory][temperature].items())]
                res[memory][temperature] = {'avg': avg, 'std': std}
        for memory in ('d', 'c'):
            temperature = '-'
            tmp = [e[1] for e in sorted(res[memory][temperature].items())]
            columns = sorted(res[memory][temperature].keys())
            res[memory][temperature] = dict()
            for index, col in enumerate(columns):
                res[memory][temperature][col] = {'avg': np.mean(tmp[index]), 'std': np.std(tmp[index])}
            avg = [e[1]['avg'] for e in sorted(res[memory][temperature].items())]
            std = [e[1]['std'] for e in sorted(res[memory][temperature].items())]
            res[memory][temperature] = {'avg': avg, 'std': std}
        res = {f'v{i}': res}
        values = {**values, **res}

    labels_str = [2 ** i for i in range(5)]

    x = np.arange(len(labels_str))  # the label locations
    width = 0.15  # the width of the bars

    fig, ax = plt.subplots(figsize=figsize)
    ax.plot([-0.5, len(labels_str) - 0.5], [1.0, 1.0], label='ROW', color=cmap(rescale(0)))
    ax.bar(x + -1.0 * width * 1.5, np.divide(values['v4']['c']['-']['avg'], values['v4']['d']['-']['avg']), width * 1.5,
           yerr=np.divide(values['v4']['c']['-']['std'], values['v4']['d']['-']['avg']), color=cmap(rescale(3)),
           edgecolor='black', label='COL')
    ax.bar(x + 0.0 * width * 1.5, np.divide(values['v4']['r']['c']['avg'], values['v4']['d']['-']['avg']), width * 1.5,
           yerr=np.divide(values['v4']['r']['c']['std'], values['v4']['d']['-']['avg']), color=cmap(rescale(1)),
           edgecolor='black', label='RME Cold')
    ax.bar(x + 1.0 * width * 1.5, np.divide(values['v4']['r']['h']['avg'], values['v4']['d']['-']['avg']), width * 1.5,
           yerr=np.divide(values['v4']['r']['h']['std'], values['v4']['d']['-']['avg']), color=cmap(rescale(2)),
           edgecolor='black', label='RME Hot')

    plt.xlabel('Column width in Bytes')

    # axis 1
    ax.set_ylim([0.0, 1.1])
    ax.set_ylabel('Norm. exec. time')
    ax.set_xticks(x)
    ax.set_xlim([-0.5, 4.5])
    ax.set_xticklabels(labels_str)
    plt.rcParams['legend.handlelength'] = 1.43
    plt.rcParams['legend.handleheight'] = 1.125
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), frameon=False, ncol=4, prop={'size': 12.5})

    plt.tight_layout()

    plt.show()


def plot(refresh=False):
    generate_full_plot((6.5, 3), True)


if __name__ == '__main__':
    plot()
