import matplotlib.pyplot as plt
import numpy as np
import csv
from matplotlib.ticker import FormatStrFormatter

percentile = 95
revisions = (0, 1, 3)
offsets = 61
colors = ["red", "green", "blue", "purple"]
revision_names = ["BSL", "PCK", None, "MLP"]

def read(filepath, skip):
    res = {} #[[] for i in range(offsets)]
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
#            bench,mem,temp,row_size,row_count,col_width,perf_l1_total,perf_l1_miss,perf_l2_total,perf_l2_miss
        for bench, mem, temp, row_size, row_count, col_width, l1_total, l1_miss, l2_total, l2_miss in csv_reader:
            if(not (mem.strip() == 'd' and temp.strip() == 'h')):
                res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l1_miss", dict()).setdefault(int(col_width), []).append(int(l1_miss))
                res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l1_total", dict()).setdefault(int(col_width), []).append(int(l1_total))
                res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l2_miss", dict()).setdefault(int(col_width), []).append(int(l2_miss))
                res.setdefault((mem.strip(), temp.strip()), dict()).setdefault("l2_total", dict()).setdefault(int(col_width), []).append(int(l2_total))
    return res

def format_name(string):
    return (string[0].upper()+string[1]+' '+string[3].upper()+string[4:]).replace("Total", "Requests")

def generate_plot(figsize):
    plt.clf()
    values = read("q1_v4.csv", 1)

    for config in (('d', '-'), ('r', 'h'), ('r', 'c'), ('c', '-')):
        for perf in ("l1_total", "l1_miss", "l2_total", "l2_miss"):
            values[config][perf] = [{'avg' : np.mean(values[config][perf][col]), 'std' : np.std(values[config][perf][col])} for col in sorted(values[config][perf].keys())]
            values[config][perf] = {'avg' : [values[config][perf][col]['avg'] for col in range(len(values[config][perf]))], 'std' : [values[config][perf][col]['std'] for col in range(len(values[config][perf]))]}

    labels = [2**i for i in range(0, 5)]

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.2  # the width of the bars


    fig, axs = plt.subplots(1, 2, sharex=True, figsize=figsize)
    for i in range(0, 2): #, perf in enumerate(("l1_total", "l1_miss", "l2_total", "l2_miss")):
        title = ("L1", "L2")[i]
        perf = (("l1_total", "l1_miss"), ("l2_total", "l2_miss"))[i]
        print(title, perf)
        axs[i].bar(x + -1.5*width, 100*np.divide(values[('d', '-')][perf[1]]['avg'], values[('d', '-')][perf[0]]['avg']), width, label="Direct Row-wise", color="tab:red")
        axs[i].bar(x + -0.5*width, 100*np.divide(values[('c', '-')][perf[1]]['avg'], values[('c', '-')][perf[0]]['avg']), width, label="Direct Columnar", color="tab:gray")
        axs[i].bar(x +  0.5*width, 100*np.divide(values[('r', 'c')][perf[1]]['avg'], values[('r', 'c')][perf[0]]['avg']), width, label="RME Cold", color="tab:blue")
        axs[i].bar(x +  1.5*width, 100*np.divide(values[('r', 'h')][perf[1]]['avg'], values[('r', 'h')][perf[0]]['avg']), width, label="RME Hot", color="tab:orange")

        axs[i].set_title(title)
        if (i == 1):
            axs[i].yaxis.tick_right()
        #axs[i][j].ticklabel_format(useOffset=False, axis="y", style="sci", scilimits=(2,2))
        axs[i].set_xticks(x)
        axs[i].set_xticklabels(labels)

    axs[0].set_ylabel('Miss ratio (%)')
    handles, labelsbis = axs[0].get_legend_handles_labels()
    plt.legend(handles, labelsbis, loc="lower center", fancybox=False, shadow=False, ncol=4, bbox_to_anchor=(-0.1, -0.5), labelspacing=-0.2, fontsize=9.5)
#    plt.tight_layout()
    #plt.xlabel('Column width in Bytes')
    fig.text(0.5, -0.03, "Column width in Bytes", ha="center", va="center")
    fig.savefig("performance_counters.pdf", bbox_inches='tight')
#    axs[0].legend(bbox_to_anchor=(0, -1), loc=2, borderaxespad=0.)
    plt.show()

if (__name__ == '__main__'):
    generate_plot((6, 2))
