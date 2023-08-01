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
    return (string[0].upper()+string[1]+' '+string[3].upper()+string[4:]).replace("Total", "Requests")

def generate_plot(figsize):
    plt.clf()
    values = read("PLT_result_q2_col_v4_multi_col.csv", 1)

    labels = sorted(values[('d', '-')]["l1_accesses"].keys()) # get all column widths from data

    for config in (('d', '-'), ('r', 'h'), ('r', 'c')):
        for perf in ("cycles", "l1_accesses", "l1_refills", "l2_accesses", "l2_refills", "inst_retired"):
            values[config][perf] = [{'avg' : np.mean(values[config][perf][col]), 'std' : np.std(values[config][perf][col])} for col in sorted(values[config][perf].keys())]
            values[config][perf] = {'avg' : [values[config][perf][col]['avg'] for col in range(len(values[config][perf]))], 'std' : [values[config][perf][col]['std'] for col in range(len(values[config][perf]))]}

    x = np.divide(np.arange(len(labels)), 1)  # the label locations
    width = 0.2  # the width of the bars

    fig, axs = plt.subplots(1, 3, sharex=True)#, figsize=figsize)
    # Plot
    print(labels, x, values[('d', '-')]["l1_accesses"]['avg'])
    axs[0].set_title(format_name("l2_accesses"))
    baseline = values[('d', '-')]["l2_accesses"]['avg']
    axs[0].bar(x + -1.0*width, np.divide(values[('d', '-')]["l2_accesses"]['avg'], baseline), width, label="Direct Row-wise", color="tab:red")
    axs[0].bar(x +  0.0*width, np.divide(values[('r', 'c')]["l2_accesses"]['avg'], baseline), width, label="RME Cold", color="tab:blue")
    axs[0].bar(x +  1.0*width, np.divide(values[('r', 'h')]["l2_accesses"]['avg'], baseline), width, label="RME Hot", color="tab:orange")
    print(labels, x, values[('d', '-')]["l2_accesses"]['avg'])
    axs[1].set_title(format_name("l2_refills"))
    baseline = values[('d', '-')]["l2_refills"]['avg']
    axs[1].bar(x + -1.0*width, np.divide(values[('d', '-')]["l2_refills"]['avg'], baseline), width, label="Direct Row-wise", color="tab:red")
    axs[1].bar(x +  0.0*width, np.divide(values[('r', 'c')]["l2_refills"]['avg'], baseline), width, label="RME Cold", color="tab:blue")
    axs[1].bar(x +  1.0*width, np.divide(values[('r', 'h')]["l2_refills"]['avg'], baseline), width, label="RME Hot", color="tab:orange")
    print(labels, x, values[('d', '-')]["inst_retired"]['avg'])
    axs[2].set_title("IPC")
    cpi = np.divide(values[('d', '-')]["inst_retired"]['avg'], values[('d', '-')]["cycles"]['avg'])
    baseline = cpi
    axs[2].bar(x + -1.0*width, np.divide(cpi, baseline), width, label="Direct Row-wise", color="tab:red")
    cpi = np.divide(values[('r', 'c')]["inst_retired"]['avg'], values[('r', 'c')]["cycles"]['avg'])
    axs[2].bar(x +  0.0*width, np.divide(cpi, baseline), width, label="RME Cold", color="tab:blue")
    cpi = np.divide(values[('r', 'h')]["inst_retired"]['avg'], values[('r', 'h')]["cycles"]['avg'])
    axs[2].bar(x +  1.0*width, np.divide(cpi, baseline), width, label="RME Hot", color="tab:orange")
    # general parameters
    axs[0].set_xticks(x)
    axs[0].set_xticklabels(labels)

    #axs[0].legend(ncol=3)
    #axs[0][0].set_ylabel('Amount')
    fig.text(0.02, 0.5, "Amount", ha="left", va="center", rotation=90)
    fig.text(0.5, -0.03, "Column width in Bytes", ha="center", va="center")
    #
#    plt.subplots_adjust(wspace=0.05, hspace=0)
    handles, labelsbis = axs[0].get_legend_handles_labels()
    plt.legend(handles, labelsbis, loc="lower center", fancybox=False, shadow=False, ncol=4, bbox_to_anchor=(-0.75, -0.17))
#    plt.tight_layout()
#    plt.subplots_adjust(wspace=0.05, hspace=0.45)
    plt.savefig("performance_counters.pdf", bbox_inches='tight')

if (__name__ == '__main__'):
    generate_plot((6, 2))
