# Relational Memory: A Hardware-Software Co-design for Accelerating Database Operations

This project explores a hardware-software co-design approach for accelerating database operations, with a primary focus on the concept of relational memory. It investigates the implications of relational memory for query processing and overall system performance. This repository serves as a central hub for the research, containing the codebase, experimental results, and related publications.

## Directory Structure

This project is organized into the following main directories:

*   **EDBT/**: Contains materials related to the EDBT publication (see "Publications" section). This includes source code for experiments (C programs for queries, utilities, etc.), hardware bitstreams (`EDBT/bitstreams/`), scripts for plotting results, and experiment configuration files. It also contains specific `INSTRUCTIONS.md` and `README.md` files.
*   **RISE/**: Contains materials related to the RISE publication. This includes C source code for database operations, hardware bitstreams (`RISE/bitstreams/`), Python scripts for data analysis and plotting, experiment results, and shell scripts for running experiments.
*   **vldb2023/**: Contains materials related to the VLDB 2023 demonstration (see "Publications" section). This includes C source code for queries, Python scripts for plotting, a Jupyter notebook demo (`vldb2023/demo.ipynb`), experiment results, and specific `INSTRUCTIONS.md`, `README.md`, and `RUNNING.md` files.

## Building and Running the Code

This section provides general instructions for building and running the code. Refer to the specific documentation within each subdirectory for more detailed instructions and experiment-specific configurations.

### Prerequisites

*   **Hardware:** Experiments are designed for the Xilinx Zynq UltraScale+ MPSoC platform. Ready-to-deploy bitstreams are provided in the respective `bitstreams` directories (e.g., `EDBT/bitstreams/`, `RISE/bitstreams/`).
*   **Compiler:** An AArch64 GCC (e.g., `aarch64-linux-gnu-gcc`) is generally required for compiling the C code for the target platform. For the `vldb2023` demonstration, a standard `gcc` can be used for its `query.c` program.
*   **Vivado:** Vivado 2017.4 was used for the EDBT project; later versions might be compatible.
*   **Python:** Python is used for plotting and data analysis scripts. The `vldb2023/demo.ipynb` Jupyter notebook specifically requires `jupyterlab`.

### Compilation

*   **EDBT/**:
    *   Navigate to the `EDBT/` directory.
    *   Run `make` to build the executable `rel_mem_bench`. (Note: The `EDBT/INSTRUCTIONS.md` mentions running make in a `src` subdirectory, but the `Makefile` is located directly in `EDBT/`.)
*   **RISE/**:
    *   Navigate to the `RISE/` directory.
    *   Run `make`. Executables (e.g., `db_generate`, `q1_col1`) will be placed in the `RISE/objects/` directory.
*   **vldb2023/**:
    *   Navigate to the `vldb2023/` directory.
    *   Compile the query code using: `gcc query.c -o query`

### Execution

*   **General:**
    *   Compiled executables typically need to be copied to the target hardware platform.
    *   Specific bitstreams may need to be loaded onto the FPGA.
*   **EDBT/**:
    *   The `rel_mem_bench` executable accepts various command-line options (use `-h` for details) or a `config` file.
    *   Refer to `EDBT/INSTRUCTIONS.md` for examples. Plotting scripts are in `EDBT/plotting/`.
*   **RISE/**:
    *   Executables are in `RISE/objects/`. Shell scripts for running experiments are in `RISE/sources_bash/`.
    *   Analysis and plotting scripts are in `RISE/data_analysis/` and `RISE/plotting/`.
*   **vldb2023/**:
    *   Run the `query` executable with parameters detailed in `vldb2023/RUNNING.md`.
    *   The `vldb2023/demo.ipynb` Jupyter notebook provides an interactive demonstration.

## Running Experiments and Analyzing Results

This section outlines the general workflow for evaluations. Consult subdirectory documentation for specifics.

1.  **Setup:** Configure the hardware (Xilinx Zynq UltraScale+ MPSoC) and load the appropriate bitstream.
2.  **Data Preparation:** Generate or load datasets as needed. Utilities like `db_generate` (in `RISE/objects/`) may be available.
3.  **Execution:** Run experiments using compiled executables with command-line arguments or provided shell scripts.
    *   **EDBT:** Use `rel_mem_bench` as per `EDBT/INSTRUCTIONS.md`.
    *   **RISE:** Use shell scripts in `RISE/sources_bash/` or executables from `RISE/objects/`.
    *   **vldb2023:** Use the `query` executable as per `vldb2023/RUNNING.md` or the `demo.ipynb`.
4.  **Data Collection:** Results (performance metrics, query outputs) are often saved to CSV files (e.g., in `EDBT/plotting/data/`, `RISE/plotting/.../results_framing.csv`, `vldb2023/results/`) or printed to standard output.
5.  **Analysis & Visualization:** Use Python scripts to process data and generate plots.
    *   **EDBT:** Scripts like `plot_projectivity.py` are in `EDBT/plotting/`. Plots may be saved in `EDBT/plotting/plots/`.
    *   **RISE:** Scripts are in `RISE/plotting/` subdirectories and `RISE/data_analysis/`.
    *   **vldb2023:** Scripts are in `vldb2023/plotting/`. The `demo.ipynb` also includes visualizations.

## Publications

This research has led to the following publications:

*   **EDBT 2023:** Relational Memory: Native In-Memory Accesses on Rows and Columns.
    *   [BU DiSC Lab](https://disc.bu.edu/papers/edbt23-relational-memory)
    *   [EDBT 2023 Proceedings](https://openproceedings.org/2023/conf/edbt/paper-177.pdf)
*   **VLDB 2023 Demo:** On-the-fly Data Transformation in Action.
    *   [BU DiSC Lab](https://disc.bu.edu/papers/vldb23-mun)
    *   [VLDB 2023 Proceedings](https://www.vldb.org/pvldb/volumes/16/paper/On-the-fly%20Data%20Transformation%20in%20Action)

(Information for the RISE publication should be added here when available.)

## Contributing

We welcome contributions! Please follow these general guidelines:

1.  **Fork & Branch:** Fork the repository and create a descriptive branch for your changes.
2.  **Coding Style:**
    *   **C:** Follow existing style if discernible; otherwise, aim for K&R.
    *   **Python:** Adhere to PEP 8.
    *   Comment complex logic.
3.  **Tests:**
    *   Add tests for new features or bug fixes.
    *   The `EDBT/tests/` directory contains examples. Check other subdirectories for any existing test infrastructure.
4.  **Compile & Test:** Ensure your changes compile and all tests pass.
5.  **Pull Request:** Submit a PR with a clear title and detailed description of your changes.

Open an issue to discuss significant contributions or questions.
