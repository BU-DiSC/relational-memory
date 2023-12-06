import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_data(df, ax, y_max):
    # Group data and calculate means and standard deviations
    grouped_mean = df.groupby(['enabled_col_num', 'mem', 'temp'])['cycles'].mean().unstack(level=[1, 2]).fillna(0)
    grouped_std = df.groupby(['enabled_col_num', 'mem', 'temp'])['cycles'].std().unstack(level=[1, 2]).fillna(0)
    
    # Plotting
    x = sorted(df['enabled_col_num'].unique())
    width = 0.2
    bar_positions = np.arange(len(x))
    offset = width * np.array([-1.5, -0.5, 0.5, 1.5])

    bars_d = ax.bar(bar_positions + offset[0], grouped_mean['d']['-'], width, yerr=grouped_std['d']['-'], label='ROW', color='black', capsize=5)
    bars_c = ax.bar(bar_positions + offset[1], grouped_mean['c']['-'], width, yerr=grouped_std['c']['-'], label='COL', color='white', edgecolor='black', capsize=5)
    bars_r_c = ax.bar(bar_positions + offset[2], grouped_mean['r']['c'], width, yerr=grouped_std['r']['c'], label='RME Cold', color='#696969', capsize=5)
    bars_r_h = ax.bar(bar_positions + offset[3], grouped_mean['r']['h'], width, yerr=grouped_std['r']['h'], label='RME Hot', color='#C0C0C0', capsize=5)

    ax.set_ylabel('Average CPU Cycles')
    ax.set_xticks(bar_positions)
    ax.set_xticklabels(x)
    ax.set_xlabel('Number of Enabled Columns')
    ax.set_ylim([0, y_max])

def find_global_max(df):
    return df['cycles'].max()

def plot_and_save(input_csv, output_file, main_title, secondary_title):
    df = pd.read_csv(input_csv)

    df.columns = [col.strip() for col in df.columns]
    df['mem'] = df['mem'].str.strip()
    df['temp'] = df['temp'].str.strip()

    global_max = find_global_max(df)

    fig, ax = plt.subplots(1, 1, figsize=(12, 6))

    plot_data(df, ax, global_max)

    fig.suptitle(main_title, fontweight='bold', fontsize=16)
    ax.set_title(secondary_title, fontweight='bold', fontsize=14)

    handles, labels = ax.get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=4)

    fig.tight_layout()
    fig.savefig(output_file, bbox_inches='tight', format='png')
    plt.close(fig)

plot_and_save('data/projectivity/result_projectivity.csv', 'plots/projectivity.png', 'Projectivity', 'Q1')
