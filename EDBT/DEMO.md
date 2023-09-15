# Demonstration Scenarios

## S1. Overhead of Fetching Data through RME

- running the experiment
> \# ./sources_bash/experiment_q1.sh
>
> cd ~/sources_bash
>
> bash experiment_q1.sh
>
> \# copy the result file to the host machine PLT_result_q1_col.csv

- creating the figure
> cd ./plotting/queries_experiment/
>
> python3 demo.py

## S2. MVCC Transactions

- running the experiment
> \# ./sources_bash/experiment_mvcc.sh
>
> cd ~/sources_bash
>
> bash experiment_mvcc.sh
>
> \# copy the result file to the host machine PLT_result_mvcc.csv

- creating the figures
> cd ./plotting/projectivity/
>
> python3 mvcc.py

## S3. Scalability of RME

- running the experiment
> \# ./sources_bash/experiment_scale.sh
>
> cd ~/sources_bash
>
> bash experiment_scale.sh
>
> \# copy the result file to the host machine result_scale.csv

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
