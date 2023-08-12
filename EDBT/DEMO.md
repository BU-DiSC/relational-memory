# Demonstration Scenarios

## S1. Overhead of Fetching Data through RME

- running the experiment
> \# ./sources_bash/experiment_q1.sh
>
> cd ~/sources_bash
>
> bash experiment_q1.sh

- creating the figure
> cd ./plotting/queries_experiment/
>
> python3 plot.py

## S2. MVCC Transactions

- running the experiment
> \# ./sources_bash/experiment_mvcc.sh
>
> cd ~/sources_bash
>
> bash experiment_mvcc.sh

- creating the figures
> cd ./plotting/queries_experiment/
>
> python3 plot.py
>
> python3 mvcc.py

## S3. Scalability of RME

- running the experiment
> \# ./sources_bash/experiment_scale.sh
>
> cd ~/sources_bash
>
> bash experiment_scale.sh

- creating the figures
> cd ./plotting/scale
>
> python3 plot.py

## S4. Join Queries

- running the experiment
> \# ./sources_bash/experiment_q5_v_col_widths.sh
>
> \# ./sources_bash/experiment_q5_v_row_size.sh
>
> cd ~/sources_bash
>
> bash experiment_q5_v_col_widths.sh
>
> bash experiment_q5_v_row_size.sh

- creating the figures
> cd ./plotting/join
>
> python3 plot.py