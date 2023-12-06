# Set up

1. Run `make` in the `src` directory.
2. Copy the generated executable `new_demo` to the board.

# Configuration and Execution Guide

## Default Configuration

- The program initially sets a [Default Configuration](utils/exp_config.c) for the experiment. These defaults are hard-coded and provide baseline settings for the database and experiment. You can override the default configuration with command-line options, or by adding a "config" file.

## Command-Line Arguments

### Usage

- Run the program with the `-r` option followed by the query name and additional options as needed.
- Example: `./new_demo -r q2 [options]`
- For projectivity experiment, use `-p`.
- For single query mode, use `-q` followed by the query name and options.

### Execute the Program

- Example: Run q2 in single query mode.
  `./new_demo -q q2 -L -O 0,4 -K 136`
- Example: Run the row size experiment with q2, number of experiments 1.
  `./new_demo -r q2 -N 1`
- Example: Run the projectivity experiment for q1.
  `./new_demo -p`

### Options

- `-L`: Load database configuration from the 'config' file.
- `-C`: Specify the number of columns.
- `-R`: Set the number of row counts.
- `-N`: Define the number of experiment samples.
- `-O`: Specify column offsets (comma-separated).
- `-K`: Set the K value for the query.
- `-S`: Choose storage type (r: row store, c: column store).
- `-T`: Define column types (s: sorted, r: random, z: zero-padded).
- `-P`: Enable printing of the created database and query results.
- `-m`: Set the minimum value.
- `-M`: Set the maximum value.
- `-V`: Enable MVCC mode.
   
## Configuration File

### Customizing Configuration

- Customize the program's configuration by creating a "config" file in the program directory.
- This file allows setting various parameters to override the default settings.

### Using the Configuration File

- If a "config" file is found, the program reads and applies these settings.
- If the file is not found, the program proceeds with the default configuration, and a warning message is displayed.

# Fine-Tuning Configuration

For more direct control, override both the default and file-based configurations using command-line arguments when running the program.
