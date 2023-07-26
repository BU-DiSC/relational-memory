import matplotlib.pyplot as plt
from scipy.stats import hmean
from  matplotlib.colors import LinearSegmentedColormap
from  matplotlib import colors 
from matplotlib import gridspec, projections
import seaborn as sns
import pandas as pd
import matplotlib as mpl

percentile = 95
revisions = 4
offsets = 60

def read(filepath, skip):
    print(filepath)
    res = {}
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        for bench, mem, temp, proj_col, sel, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired in csv_reader:
            res.setdefault(mem.strip(),dict()).setdefault(int(proj_col.strip()), dict()).setdefault(float(sel.strip()), []).append(int(cycles))
            #res.setdefault(mem.strip(),dict()).setdefault(int(proj_col.strip()), []).append(int(cycles))
    return res

def generate_full_plot( type, figsize):
    plt.clf()
    plt.rcParams.update({'font.size': 16})
    if type != 'row' and type != 'col' :
        print("It should be 'row' or 'col'")
        return
    
    if type=='row' :
        type_char = 'd'
    elif type=='col' :
        type_char = 'c'


    csv_file = "heatmap_relmem_"+type+"_store.csv"
    csv_reader = pd.read_csv(csv_file, delimiter=',')
    
    rel = []

    # x --> proj, y --> sel
    x_labels = range(1, 11, 1)
    y_labels = range(1, 11, 1)


    fig, ax = plt.subplots(figsize=figsize)
    if type =='row':
        colormap = plt.cm.get_cmap('Greens')
        ax = sns.heatmap( csv_reader, annot=True, xticklabels=x_labels, yticklabels=y_labels, cmap=colormap, fmt=".3", annot_kws={"size":10}) 
    elif type =='col':
        colormap = LinearSegmentedColormap.from_list('rb',["r", "w", "b"], N=512) 
        ax = sns.heatmap( csv_reader, vmin=0.4, vmax=2.3, annot=True, xticklabels=x_labels, yticklabels=y_labels, cmap=colormap, center=1, fmt=".3", annot_kws={"size":10})

    plt.yticks(rotation=0)
    plt.xticks(rotation=0)

    ax.invert_yaxis()

    plt.ylabel('# of Selection Columns')
    plt.xlabel('# of Projected Columns')

    plt.tight_layout()

    plt.savefig("proj_sel_"+type_char+".pdf")
    plt.show()

if (__name__ == '__main__'):
    generate_full_plot('row', (5.5, 4))
    generate_full_plot('col', (5.5, 4))
