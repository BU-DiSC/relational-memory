# Relational Memory: Native In-Memory Accesses on Rows and Columns

This work was accepted for publication in the Proceedings of the International Conference on Extending Database Technology (EDBT), 2023.

Analytical database systems are typically designed to use a columnfirst data layout to access only the desired fields. On the other
hand, storing data row-first works great for accessing, inserting, or updating entire rows. Transforming rows to columns at runtime is expensive, hence, many analytical systems ingest data in row-first form and transform it in the background to columns to
facilitate future analytical queries. How will this design change if we can always efficiently access only the desired set of columns?
To address this question, we present a radically new approach to data transformation from rows to columns. We build upon recent advancements in embedded platforms with re-programmable logic to design native in-memory access on rows and columns.
Our approach, termed Relational Memory (RM), relies on an FPGA-based accelerator that sits between the CPU and main
memory and transparently transforms base data to any group of columns with minimal overhead at runtime. This design allows
accessing any group of columns as if it already exists in memory. We implement and deploy RM in real hardware, and we show that
we can access the desired columns up to 1.63× faster compared to a row-wise layout, while matching the performance of pure
columnar access for low projectivity, and outperforming it by up to 2.23× as projectivity (and tuple reconstruction cost) increases.
Overall, RM allows the CPU to access the optimal data layout, radically reducing unnecessary data movement without high
data transformation costs, thus, simplifying software complexity and physical design, while accelerating query execution.

Usefull links:

- [Instructions](INSTRUCTIONS.md)

- [BU DiSC Lab](https://disc.bu.edu/papers/edbt23-relational-memory)

- [EDBT 2023](https://openproceedings.org/2023/conf/edbt/paper-177.pdf)
