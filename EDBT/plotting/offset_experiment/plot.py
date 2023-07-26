import matplotlib.pyplot as plt
import numpy as np
import csv
from matplotlib.ticker import FormatStrFormatter

percentile = 95
revisions = [3] #0, 1,
offsets = 61
cmap=plt.get_cmap('binary_r')
rescale = lambda x: (x - 0) / (3 - 0)
colors = [cmap(rescale(0)), cmap(rescale(1)), cmap(rescale(2)), cmap(rescale(3))]
#colors = ["tab:red", "tab:blue", "tab:orange", "tab:gray"]
revision_names = ["BSL", "PCK", None, "RME"]
marks = ["x", "^", "o", "none"]
periodmus = 0.00066666666666667 # micro second
periodms  = 0.00000066666666666667 # micro second


def read(filepath, skip):
    res = {} #[[] for i in range(offsets)]
    with open(filepath) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for x in range(0, skip):
            next(csv_reader)
        for bench, mem, temp, row_size, row_count, col_width, offset, cycles in csv_reader:
            if(not (mem.strip() == 'd' and temp.strip() == 'h')):
                res.setdefault((mem.strip(), temp.strip()), [[] for i in range(offsets)])[int(offset)].append(periodmus*int(cycles))
    return res

if (__name__ == '__main__'):
    values = [read("PLT4_result_db_latency_summing_v"+str(r+1)+".csv", 1) for r in range(4)]

    for revision in revisions:
        for config in values[revision].keys():
            for offset in range(offsets):
                values[revision][config][offset] = {'avg' : np.mean(values[revision][config][offset]), 'std' : np.std(values[revision][config][offset])}
            values[revision][config] = {'avg' : [values[revision][config][offset]['avg'] for offset in range(offsets)]}

    plt.rcParams.update({'font.size': 14})
    fig, ax1 = plt.subplots(1, 1, figsize=(6.5, 3))
#    fig.subplots_adjust(hspace=0.05)  # adjust space between axes

    for revision in revisions:
        for config in (('d','c'),('r','c'),('r','h')):
        #for config in values[revision].keys():
            if(config[0] != 'd' or revision == 3):
                revision_name = revision_names[revision]
                memory_target = "Direct Access" if(config[0]=='d') else "RelBuffer"
                temperature = "Cold" if(config[1]=='c') else ("Hot" if(memory_target=="RelBuffer") else "")
                if memory_target == "Direct Access" :
                    line_color = colors[0]
                    line_style = 'solid'
                elif memory_target == "RelBuffer" :
                    line_color = colors[1]
                    line_style = 'dashed' if (temperature == "Cold") else 'dotted'
                    #line_color = colors[1] if (temperature == "Cold") else colors[2]
             
                label_str = "ROW" if(config[0]=='d') else revision_name+" "+temperature #", "+memory_target+
                mark_str = marks[0] if (memory_target == "Direct Access") else marks[1+int(config[1]=='h')]
                ax1.plot(values[revision][config]['avg'],
                    label=label_str, 
                    marker=mark_str,
                    fillstyle='none',
                    markersize=8.5,
                    markevery=2,
                    color=line_color
                    #linestyle=line_style
#                    linestyle="dotted" if(memory_target=="Direct Access") else ("solid" if(temperature=="Cold") else "dashed")
                )
#                ax2.plot(values[revision][config]['avg'],
#                    label=label_str,
#                    color=line_color,
#                    linestyle="dotted" if(memory_target=="Direct Access") else ("solid" if(temperature=="Cold") else "dashed"))
#                ax3.plot(values[revision][config]['avg'],
#                    label=label_str,
#                    color=line_color,
#                    linestyle="dotted" if(memory_target=="Direct Access") else ("solid" if(temperature=="Cold") else "dashed"))

#    ax1.set_ylim(0)
#    ax2.set_ylim(0.68*1e7, 0.77*1e7)
#    ax3.set_ylim(0.00*1e7, 0.09*1e7)

#    ax1.spines['bottom'].set_visible(False)
#    ax2.spines['top'].set_visible(False)
#    ax1.xaxis.tick_top()
#    ax1.tick_params(labeltop=False)  # don't put tick labels at the top
#    ax2.xaxis.tick_bottom()

#    ax1.spines['bottom'].set_visible(False)
#    ax2.spines['top'].set_visible(False)
#    ax2.spines['bottom'].set_visible(False)
#    ax3.spines['top'].set_visible(False)
#    ax1.tick_params(axis='x', length=0)
#    ax2.tick_params(axis='x', length=0)

#    ax1.tick_params(labeltop=False)
#    ax2.tick_params(labeltop=False)
#    ax3.tick_params(labeltop=False)
#    ax3.xaxis.tick_bottom()

#    d = .5
#    kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12, linestyle="none", color='k', mec='k', mew=1, clip_on=False)
#    ax1.plot([0, 1], [0, 0], transform=ax1.transAxes, **kwargs)
#    ax2.plot([0, 1], [1, 1], transform=ax2.transAxes, **kwargs)
#    ax2.plot([0, 1], [0, 0], transform=ax2.transAxes, **kwargs)
#    ax3.plot([0, 1], [1, 1], transform=ax3.transAxes, **kwargs)

#    ax1.yaxis.set_major_formatter(plt.ScalarFormatter(useMathText=True))
#    ax1.ticklabel_format(useOffset=False, axis="y", style="sci", scilimits=(7,7))
#    ax2.yaxis.set_major_formatter(plt.ScalarFormatter(useMathText=True))
#    ax2.ticklabel_format(useOffset=False, axis="y", style="sci", scilimits=(7,7))
#    ax3.yaxis.set_major_formatter(plt.ScalarFormatter(useMathText=True))
#    ax3.ticklabel_format(useOffset=False, axis="y", style="sci", scilimits=(7,7))

    #plt.legend(loc='upper center', bbox_to_anchor=(0.45, -0.37), fancybox=False, shadow=False, ncol=3)
    plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.05), frameon=False, ncol=3)
    plt.xlabel("Offset within each row")
#    ax1.set_xticks([])
#    ax2.set_xticks([])
    ax1.set_xticks([i for i in range(0, offsets, 4)])
    ax1.set_ylim(290, 630)

#    ax1.set_yticks([i for i in range(int(1.18*1e7), int(1.21*1e7), int(0.01*1e7))])
#    ax2.set_yticks([i for i in range(int(0.70*1e7), int(0.74*1e7), int(0.01*1e7))])
#    ax3.set_yticks([i for i in range(int(0.04*1e7), int(0.10*1e7), int(0.01*1e7))])
    ax1.set_ylabel("Execution time (\u03BCs)")

    plt.tight_layout()
    plt.savefig("offset.pdf", bbox_inches='tight')

    plt.show()
