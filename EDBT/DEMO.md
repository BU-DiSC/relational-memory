# Demonstration Scenarios

## S1. Overhead of Fetching Data through RME

- running the experiment
> ./sources_bash/experiment_q1.sh
- creating the figure
> python3 ./plotting/queries_experiment/plot.py

## S2. MVCC Transactions

- running the experiment
> ./sources_bash/experiment_mvcc.sh
- creating the figures
> python3 ./plotting/projectivity/plot.py
>
> python3 ./plotting/projectivity/mvcc.py

## S3. Scalability of RME

- running the experiment
> ./sources_bash/experiment_tpch.sh
- creating the figures
> python3 ./plotting/tpch/plot.py

## S4. Join Queries

- running the experiment
> ./sources_bash/experiment_q5_v_col_widths.sh
>
> ./sources_bash/experiment_q5_v_row_size.sh
- creating the figures
> python3 ./plotting/join/plot.py