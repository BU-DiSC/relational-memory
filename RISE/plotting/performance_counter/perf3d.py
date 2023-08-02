import matplotlib.pyplot as plt
import numpy as np
import csv
from copy import deepcopy
from matplotlib.ticker import FormatStrFormatter
from mpl_toolkits.mplot3d import Axes3D

percentile = 95
revisions = (0, 1, 3)
offsets = 61
colors = ["red", "green", "blue", "purple"]
revision_names = ["BSL", "PCK", None, "MLP"]

def read(filepath, skip):
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        for bench, mem, temp, row_size, row_count, col_width, cycles, l1_accesses, l1_refills, l2_accesses, l2_refills, inst_retired in csv_reader:
            res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("cycles", dict()).setdefault(int(col_width), []).append(int(cycles))
            res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l1_accesses", dict()).setdefault(int(col_width), []).append(int(l1_accesses))
            res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l1_refills", dict()).setdefault(int(col_width), []).append(int(l1_refills))
            res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l2_accesses", dict()).setdefault(int(col_width), []).append(int(l2_accesses))
            res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l2_refills", dict()).setdefault(int(col_width), []).append(int(l2_refills))
            res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("inst_retired", dict()).setdefault(int(col_width), []).append(int(inst_retired))
    return res

def format_name(string):
    return string #(string[0].upper()+string[1]+' '+string[3].upper()+string[4:]).replace("Total", "Requests")

def generate_plot(figsize):
    plt.clf()
    queries = ["Q1", "Q2", "Q3", "Q4"]
    values = read("PLT_result_q2_col_v4_multi_col.csv", 1)

    labels = sorted(values[('d', '-')]["l1_accesses"].keys()) # get all column widths from data
    labels.reverse()

    for config in (('d', '-'), ('r', 'h'), ('r', 'c')):
        for perf in ("cycles", "l1_accesses", "l1_refills", "l2_accesses", "l2_refills", "inst_retired"):
            values[config][perf] = [{'avg' : np.mean(values[config][perf][col]), 'std' : np.std(values[config][perf][col])} for col in sorted(values[config][perf].keys())]
            values[config][perf] = {'avg' : [values[config][perf][col]['avg'] for col in range(len(values[config][perf]))], 'std' : [values[config][perf][col]['std'] for col in range(len(values[config][perf]))]}
        values[config]["IPC"] = dict()
        values[config]["IPC"]["avg"] = list(np.divide(values[config]["inst_retired"]["avg"], values[config]["cycles"]["avg"]))

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    y = np.divide(np.arange(len(queries)), 1)
    width = 0.2  # the width of the bars

    for perf in ("cycles", "l1_accesses", "l1_refills", "l2_accesses", "l2_refills", "inst_retired", "IPC"):
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1, projection='3d')
        # Plot normalized lines
        ax.plot3D([i/10 for i in range(-5, 50, 5)], [3.25 for i in range(-5, 50, 5)], [1 for i in range(-5, 50, 5)], 'tab:red', label="Direct Row-wise")
        ax.plot3D([-0.5 for i in range(-25, 350, 25)], [i/100 for i in range(-25, 350, 25)], [1 for i in range(-25, 350, 25)], 'tab:red')
        # Plot bars
        print(labels, x, values[('d', '-')][perf]['avg'])
        baseline = deepcopy(values[('d', '-')][perf]['avg'])
        baseline.reverse()
        once = False
        for i, query in enumerate(queries):
    #        for offset, config, color, label in zip((-0.5, 0.5, 1.5), (('d', '-'), ('r', 'h'), ('r', 'c')), ("tab:red", "tab:blue", "tab:orange"), ("Direct Row-wise", "RME Cold", "RME Hot")):
            for offset, config, color, label in zip((0, 1), (('r', 'h'), ('r', 'c')), ("tab:blue", "tab:orange"), ("RME Cold", "RME Hot")):
                data = deepcopy(values[config][perf]['avg'])
                data.reverse()
                if (once):
                    ax.bar(x + offset*width, np.divide(data, baseline), width=width, zs=i, zdir='y', color=color, alpha=0.6)
                else:
                    ax.bar(x + offset*width, np.divide(data, baseline), width=width, zs=i, zdir='y', label=label, color=color, alpha=0.6)
            once = True
        # general parameters
        ax.set_xticks(x)
        ax.set_xticklabels(labels)
        ax.set_yticks(y)
        ax.set_yticklabels(queries)

        ax.set_xlim([ -0.5,  4.5])
        ax.set_ylim([-0.25, 3.25])

        ax.set_xlabel("Column width (Bytes)")
        ax.set_ylabel("Query")
        ax.set_zlabel("Normalized "+format_name(perf))

        ax.legend()
        plt.savefig("perf3d_"+perf+".pdf")
        plt.clf()

if (__name__ == '__main__'):
    generate_plot((6, 2))
