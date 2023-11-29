import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_data(df, ax, version_label, y_max):
    # Group data and calculate means and standard deviations
    grouped_mean = df.groupby(['row_size', 'mem', 'temp'])['cycles'].mean().unstack(level=[1, 2]).fillna(0)
    grouped_std = df.groupby(['row_size', 'mem', 'temp'])['cycles'].std().unstack(level=[1, 2]).fillna(0)
    
    # Plotting
    x = sorted(df['row_size'].unique())
    width = 0.2  # Adjusted width to fit the new bar
    bar_positions = np.arange(len(x))  # Base positions for bars
    offset = width * np.array([-1.5, -0.5, 0.5, 1.5])  # Offsets for each bar

    bars_d = ax.bar(bar_positions + offset[0], grouped_mean['d']['-'], width, yerr=grouped_std['d']['-'], label='ROW', color='black', capsize=5)
    bars_c = ax.bar(bar_positions + offset[1], grouped_mean['c']['-'], width, yerr=grouped_std['c']['-'], label='COL', color='white', edgecolor='black', capsize=5)
    bars_r_c = ax.bar(bar_positions + offset[2], grouped_mean['r']['c'], width, yerr=grouped_std['r']['c'], label='RME Cold', color='#696969', capsize=5)
    bars_r_h = ax.bar(bar_positions + offset[3], grouped_mean['r']['h'], width, yerr=grouped_std['r']['h'], label='RME Hot', color='#C0C0C0', capsize=5)

    ax.set_ylabel('cycles')
    ax.set_xticks(bar_positions)
    ax.set_xticklabels(x)
    ax.set_xlabel('Row Size')
    ax.set_ylim([0, y_max])  # Set y-axis limits

    # Add version label
    ax.text(0.5, 1.02, version_label, transform=ax.transAxes, fontsize=12, fontweight='bold', va='center', ha='center')

def find_global_max(dataframes):
    max_value = 0
    for df in dataframes:
        current_max = df['cycles'].max()
        if current_max > max_value:
            max_value = current_max
    return max_value

def plot_and_save(input_csv_1, input_csv_2, input_csv_3, output_file, query_name):
    # Reading the data
    df1 = pd.read_csv(input_csv_1)
    df2 = pd.read_csv(input_csv_2)
    df3 = pd.read_csv(input_csv_3)

    # Strip column names and clean data for all dataframes
    dataframes = [df1, df2, df3]
    for df in dataframes:
        df.columns = [col.strip() for col in df.columns]
        df['mem'] = df['mem'].str.strip()
        df['temp'] = df['temp'].str.strip()

    # Find global maximum for y-axis
    global_max = find_global_max(dataframes)

    # Create a figure with three subplots
    fig, axs = plt.subplots(1, 3, figsize=(18, 6))

    # Plot each dataset with a common y-axis limit
    plot_data(df1, axs[0], "Query 1", global_max)
    plot_data(df2, axs[1], "Query 2", global_max)
    plot_data(df3, axs[2], "Query 3", global_max)

    # Title for the entire figure
    fig.suptitle(query_name, fontweight='bold', fontsize=16)

    # Legend for the entire figure
    handles, labels = axs[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=4)

    fig.tight_layout()
    fig.savefig(output_file, bbox_inches='tight', format='png')
    plt.close(fig)

# Example usage
plot_and_save('data/row_size/PLT2_result_q1_col.csv', 'data/row_size/PLT2_result_q2_col.csv', 'data/row_size/PLT2_result_q3_col.csv', 'plots/cycles_q1_q2_q3_v5.png', 'v5')
