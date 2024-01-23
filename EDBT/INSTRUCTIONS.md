# Seting up the device

For the experiments in this repository, we used the Xilinx Zynq UltraScale+ MPSoC platform and Vivado 2017.4. Ready-to-deploy bitstreams are available in the `bitstreams` directory. You are encouraged to use the latest version, but feel free to experiment with earlier versions as well.

# Configuration and Execution Guide

1. Run `make` in the `src` directory.
2. Copy the generated executable `new_demo` to the board.

## Default Configuration

The program initially sets a [default configuration](utils/exp_config.c) for the experiment. These default values are hard-coded and provide baseline settings for the database and experiment. You can override the default configuration with command-line options, or by adding a [config](config) file. For example you can copy this [config](config) file to the same directory as `new_demo` and customize the database and experiment parameters.

### Command line Options

You can run `./new_demo -h` to see the command line options

- `-L`: Load database configuration from the 'config' file.
- `-C`: Specify the number of columns.
- `-R`: Set the number of rows.
- `-N`: Define the number of experiment samples.
- `-O`: Specify column offsets (comma-separated).
- `-K`: Set the K value for the query.
- `-S`: Choose storage type (r: row store, c: column store).
- `-T`: Define column types (s: sorted, r: random, z: zero-padded).
- `-P`: Enable printing of the created database and query results.
- `-m`: Set the minimum value.
- `-M`: Set the maximum value.

### Examples

- Run the single query with `-q` option followed by the query name such as `q2` and additional options. For example the following query would load the database configuration from the `config` file.

  ```./new_demo -q q2 -L -O 0,4 -K 136```

  This command would run query `Q2: SELECT A0 FROM S WHERE A1 > 136;`

- Run the varying row size experiment with the `-r` option followed by the query name and additional options as needed. For example    the following command would run a varying row size experiment with the hard-coded database and experiment settings but it would override the default number of experiment by `-N 1` option.

  ```./new_demo -r q2 -N 1```

  
- For projectivity experiment, use `-p` option.

  ```./new_demo -p```


### Plotting

After running the experiments the output will show you the path to the saved query results or the performance measures. For example
this [performance results](plotting/data/row_size/PLT2_result_q1_col.csv) file has the basic information about the experiment settings such as the number of rows, row size, column widths, and performance measures such a CPU cycles count, information about cache refills and number of instructions retired.

You can use the python scripts that we provide in `plotting` directory to generate the plots for projectivity and varying row-size experiments. You can take a look at the `plotting/plots` directory for sample plots.