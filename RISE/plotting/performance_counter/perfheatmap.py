import matplotlib.pyplot as plt
import numpy as np
import csv
from copy import deepcopy
from matplotlib.ticker import FormatStrFormatter
from mpl_toolkits.axes_grid1 import make_axes_locatable
from mpl_toolkits.axes_grid1 import ImageGrid

percentile = 95
revisions = (0, 1, 3)
offsets = 61
colors = ["red", "green", "blue", "purple"]
revision_names = ["BSL", "PCK", None, "MLP"]

def heatmap(data, row_labels, col_labels, title, left_most=False, ax=None, cbar_kw={}, cbarlabel="", **kwargs):
    """
    Create a heatmap from a numpy array and two lists of labels.

    Parameters
    ----------
    data
        A 2D numpy array of shape (M, N).
    row_labels
        A list or array of length M with the labels for the rows.
    col_labels
        A list or array of length N with the labels for the columns.
    ax
        A `matplotlib.axes.Axes` instance to which the heatmap is plotted.  If
        not provided, use current axes or create a new one.  Optional.
    cbar_kw
        A dictionary with arguments to `matplotlib.Figure.colorbar`.  Optional.
    cbarlabel
        The label for the colorbar.  Optional.
    **kwargs
        All other arguments are forwarded to `imshow`.
    """

    if not ax:
        ax = plt.gca()

    # Plot the heatmap
    im = ax.imshow(data, **kwargs)
    if (not left_most):
        ax.get_yaxis().set_visible(False)

    # Show all ticks and label them with the respective list entries.
    ax.set_xticks(np.arange(data.shape[1]))
    ax.set_xticklabels(row_labels, fontsize=13)
    ax.set_yticks(np.arange(data.shape[0]))
    ax.set_yticklabels(col_labels, fontsize=13)

    # Let the horizontal axes labeling appear on top.
    ax.tick_params(top=False, bottom=True, labeltop=False, labelbottom=True)
    # Rotate the tick labels and set their alignment.
    plt.setp(ax.get_xticklabels(), rotation=30, ha="right", rotation_mode="anchor", fontsize=13)

    # Turn spines off and create white grid.
    #ax.spines[:].set_visible(False)

    ax.set_title(title, fontsize=15)

    ax.set_xticks(np.arange(data.shape[1]+1)-.5, minor=True)
    ax.set_yticks(np.arange(data.shape[0]+1)-.5, minor=True)
    ax.grid(which="minor", color="w", linestyle='-', linewidth=0)
    ax.tick_params(which="minor", bottom=False, left=False)

    return im


def annotate_heatmap(im, data=None, valfmt="{x:.2f}", textcolors=("black", "white"), threshold=None, **textkw):
    """
    A function to annotate a heatmap.

    Parameters
    ----------
    im
        The AxesImage to be labeled.
    data
        Data used to annotate.  If None, the image's data is used.  Optional.
    valfmt
        The format of the annotations inside the heatmap.  This should either
        use the string format method, e.g. "$ {x:.2f}", or be a
        `matplotlib.ticker.Formatter`.  Optional.
    textcolors
        A pair of colors.  The first is used for values below a threshold,
        the second for those above.  Optional.
    threshold
        Value in data units according to which the colors from textcolors are
        applied.  If None (the default) uses the middle of the colormap as
        separation.  Optional.
    **kwargs
        All other arguments are forwarded to each call to `text` used to create
        the text labels.
    """

    if not isinstance(data, (list, np.ndarray)):
        data = im.get_array()

    # Normalize the threshold to the images color range.
    if threshold is not None:
        threshold = im.norm(threshold)
    else:
        threshold = im.norm(data.max())/2.

    # Set default alignment to center, but allow it to be
    # overwritten by textkw.
    kw = dict(horizontalalignment="center", verticalalignment="center")
    kw.update(textkw)

#    # Get the formatter in case a string is supplied
#    if isinstance(valfmt, str):
#        valfmt = plt.ticker.StrMethodFormatter(valfmt)

    # Loop over the data and create a `Text` for each "pixel".
    # Change the text's color depending on the data.
    texts = []
    for i in range(data.shape[0]):
        for j in range(data.shape[1]):
            kw.update(color=textcolors[int(im.norm(data[i, j]) > threshold)])
            text = im.axes.text(j, i, str(round(data[i, j], 2)), fontsize=13, **kw)
            texts.append(text)

    return texts

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
    queries = ["Q1", "Q2", "Q4", "Q5"]
    values = {
        "Q1":    read("../../results_edbt/PLT1_result_q1_col.csv", 1),
        "Q2":    read("../../results_edbt/PLT1_result_q2_col.csv", 1),
        "Q3":    read("../../results_edbt/PLT1_result_q3_col.csv", 1),
        "Q4":    read("../../results_edbt/PLT1_result_q4_col.csv", 1),
        "Q5":    read("../../results_edbt/PLT1_result_q5_col.csv", 1),
        "TPCH1": read("../tpch/PLT_result_tpch_q1.csv", 1),
        "TPCH6": read("../tpch/PLT_result_tpch_q6.csv", 1)
    }
    configs = {
        "Q1":    (('d', '-'), ('c', '-'), ('r', 'h'), ('r', 'c')),
        "Q2":    (('d', '-'), ('c', '-'), ('r', 'h'), ('r', 'c')),
        "Q3":    (('d', '-'), ('c', '-'), ('r', 'h'), ('r', 'c')),
        "Q4":    (('d', '-'), ('c', '-'), ('r', 'h'), ('r', 'c')),
        "Q5":    (('d', '-'), ('c', '-'),             ('r', 'c')),
        "TPCH1": (('d', '-'), ('c', '-'),             ('r', 'c')),
        "TPCH6": (('d', '-'), ('c', '-'),             ('r', 'c'))
    }
    labels = {
        ('d', '-'): "Row-wise",
        ('c', '-'): "Columnar",
        ('r', 'h'): "RME Hot",
        ('r', 'c'): "RME Cold"
    }

    y = sorted(values["Q1"][('d', '-')]["l1_accesses"].keys())

    for query in queries:
        for config in configs[query]:
            for perf in ("cycles", "l1_accesses", "l1_refills", "l2_accesses", "l2_refills", "inst_retired"):
                values[query][config][perf] = [{'avg' : np.mean(values[query][config][perf][col]), 'std' : np.std(values[query][config][perf][col])} for col in sorted(values[query][config][perf].keys())]
                values[query][config][perf] = {'avg' : [values[query][config][perf][col]['avg'] for col in range(len(values[query][config][perf]))], 'std' : [values[query][config][perf][col]['std'] for col in range(len(values[query][config][perf]))]}
                values[query][config][perf]["nor"] = np.divide(values[query][config][perf]["avg"], values[query][('d', '-')][perf]["avg"])
            values[query][config]["IPC"] = dict()
            values[query][config]["IPC"]["avg"] = list(np.divide(values[query][config]["inst_retired"]["avg"], values[query][config]["cycles"]["avg"]))
            values[query][config]["IPC"]["nor"] = np.divide(values[query][config]["IPC"]["avg"], values[query][('d', '-')]["IPC"]["avg"])


    minmax = [values[query][config][perf]["nor"][col_width] for config in configs[query] for query in queries for col_width in range(len(y)) for perf in ("l2_accesses", "l2_refills", "IPC")]
    vmin, vmax = min(minmax), max(minmax)

    # Horizontal layout
    for perf in ("cycles", "l1_accesses", "l1_refills", "l2_accesses", "l2_refills", "inst_retired", "IPC"):
        fig = plt.figure(figsize=(10, 6), dpi=300)
        ax = ImageGrid(fig, 111, nrows_ncols=(1, len(queries)), axes_pad=0.05)#, cbar_location="right", cbar_mode="single", cbar_size="10%", cbar_pad=0.1)
        for i, query in enumerate(queries):
            x = [labels[config] for config in configs[query]]
            # Transform to matrix
            matrix = np.array([[values[query][config][perf]["nor"][col_width] for config in configs[query]] for col_width in range(len(y))])
            # Plot
            im = heatmap(matrix, x, y, query, left_most=(i == 0), ax=ax[i], cmap="YlGn", vmin=vmin, vmax=vmax)
            texts = annotate_heatmap(im, valfmt="{x:.1f}")
        plt.colorbar(im, cax=ax.cbar_axes[0])
        ax[0].set_ylabel("Column width (Bytes)", fontsize=13)
        # Save figure
        plt.subplots_adjust(wspace=0.05, hspace=0)
        plt.savefig("perf3d_"+perf+"_hor.pdf")
        plt.clf()

#    # Vertical layout
#    x, y = y, x
#    for perf in ("cycles", "l1_accesses", "l1_refills", "l2_accesses", "l2_refills", "inst_retired", "IPC"):
#        minmax = [values[query][config][perf]["nor"][col_width] for config in configs[query] for query in queries for col_width in range(len(y))]
#        vmin, vmax = min(minmax), max(minmax)
#        #fig, ax = plt.subplots(1, len(queries), figsize=(10, 6), dpi=300)
#        fig = plt.figure(figsize=(10, 6), dpi=300)
#        ax = ImageGrid(fig, 111, nrows_ncols=(len(queries), 1), axes_pad=0.05)#, cbar_location="right", cbar_mode="single", cbar_size="10%", cbar_pad=0.1)
#        for i, query in enumerate(queries):
#            # Transform to matrix
#            matrix = np.array([[values[query][config][perf]["nor"][col_width] for col_width in range(len(y))] for config in configs[query]])
#            # Plot
#            im = heatmap(matrix, x, y, query, left_most=(i == 0), ax=ax[i], cmap="YlGn", vmin=vmin, vmax=vmax)
#            texts = annotate_heatmap(im, valfmt="{x:.1f}")
#        #plt.colorbar(im, cax=ax.cbar_axes[0])
#        ax[0].set_ylabel("Column width (Bytes)", fontsize=13)
#        # Save figure
#        plt.subplots_adjust(wspace=0.05, hspace=0)
#        plt.savefig("perf3d_"+perf+"_ver.pdf")
#        plt.clf()

if (__name__ == '__main__'):
    generate_plot((7, 2))
