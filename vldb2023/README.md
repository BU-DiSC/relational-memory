# On-the-fly Data Transformation in Action

This work was presented on the Demonstration Track of VLDB 2023, where it was selected for the Best Demonstration award.

Transactional and analytical database management systems (DBMS) typically employ different data layouts: row-stores for the first and column-stores for the latter. In order to bridge the requirements of the two without maintaining two systems and two (or more) copies of the data, our proposed system Relational Memory employs specialized hardware that transforms the base row table into arbitrary column groups at query execution time. This approach maximizes the cache locality and is easy to use via a simple abstraction that allows transparent on-the-fly data transformation. Here, we demonstrate how to deploy and use Relational Memory via four representative scenarios. The demonstration uses the fullstack implementation of Relational Memory on the Xilinx Zynq UltraScale+ MPSoC platform. Conference participants will interact with Relational Memory deployed in the actual platform.

Usefull links:

- [Instructions](INSTRUCTIONS.md)

- [BU DiSC Lab](https://disc.bu.edu/papers/vldb23-mun)

- [VLDB 2023](https://www.vldb.org/pvldb/volumes/16/paper/On-the-fly%20Data%20Transformation%20in%20Action)
