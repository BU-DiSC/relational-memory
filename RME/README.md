## Set up

1. Run `make` in `src`
2. Copy the generated executable `new_demo` to the board

## Configuration and Execution Guide

### Default Configuration

- The program initially sets a [Default Configuration](utils/exp_config.c) for the experiment. These defaults are hard-coded and provide baseline settings for the database and experiment. You can override the default configuration with command line options, or by adding a "config" file.

### Command Line Arguments

- **Usage:**
  - Run the program with the `-r` option followed by the query name and additional options as needed.
  - Example: `./new_demo -r q2 [options]`

- **Options:**
  - `-S`: Set the storage type (e.g., row store, column store).
  - `-C`: Specify the number of columns.
  - `-r`: Define the row size in bytes.
  - `-R`: Set the number of rows.
  - `-T`: Choose the column types (e.g., sorted, random).
  - `-P`: Enable printing of the created database.
  - `-m`: Set the minimum value for random type columns.
  - `-M`: Set the maximum value for random type columns.
  - `-N`: Define the number of experiment samples.
  - Other options as defined in your program.

 - **Execute the Program:**
   - Run the row size experiment with q2, row size 16, number of experiments 1.
     Example: `./new_demo -r q2 -R 16 -N 1`
   - Run projectivity experiment
     Example: `./new_demo -p q1 -N 1`

   
### Configuration File

- **Customizing Configuration:**
  - You can customize the program's configuration by creating a "config" file in the program directory.
  - This file allows you to set various parameters that override the default settings.

- **Using the Configuration File:**
  - If a "config" file is found, the program will read and apply these settings.
  - If the file is not found, the program will proceed with the default configuration and a warning message will be displayed.

  - **Fine-Tuning Configuration:**
  - For more direct control, you can override both the default and file-based configurations using command-line arguments when running the program.









