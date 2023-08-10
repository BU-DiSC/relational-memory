# Run query

## Average query

```sh
./query [-q] store q1 ncols widths avg_col
# E.g.:
./query ROW q1 32768 4 4,4,4,4 2
# SELECT AVG(c2) FROM S;
```
- -q: quiet [Optional]
- store: ROW|COL|RME
- ncols: number of columns
- widths: ncols comma separated values [1|2|4|8]
- avg_col: zero based index of selected column (< ncols)

## Projection & Selection query

```sh
./query [-q] store q2 ncols widths nopts options
# E.g.:
./query ROW q2 32768 4 4,4,4,4 3 0PX,1PL400,2XL200
# SELECT c0, c1 FROM S WHERE c1 < 400 AND c2 < 200;
```
- -q: quiet [Optional]
- store: ROW|COL|RME
- ncols: number of columns
- widths: ncols comma separated values [1|2|4|8]
- nopts: number of projected or selection columns (enabled columns)
- options: nopts comma separated descriptions (no need for a column description if neither projection nor selection are enabled)
    - description: cps[v]
        - c: zero based index of column with enabled options
        - p: P if projection is enabled otherwise will not be projected
        - s: L|l|G|g|E|N for <|<=|>|>=|==|!= respectively otherwise will not apply selection filter
        - v: value used for s [Optional]

**NOTE:** Only L (<) conditions have been implemented.

## Join query

```sh
./query [-q] store q3 s_ncols s_widths r_ncols r_widths s_sel r_sel s_join r_join
# E.g.:
./query ROW q3 32768 3 4,4,4 32768 3 4,4,4 1 1 0 2
# SELECT S.c1, R.c1 FROM S, R WHERE S.c0 = R.c2;
```
- -q: quiet [Optional]
- store: ROW|COL|RME
- s_ncols: number of columns for table S
- s_widths: s_ncols comma separated values for table S [1|2|4|8]
- r_ncols: number of columns for table R
- r_widths: r_ncols comma separated values for table R [1|2|4|8]
- s_sel: projected column of S
- r_sel: projected column of R
- s_join: join column of S
- r_join: join column of R

**NOTE:** Column widths of tables S and R are disregarded and replaced with 4.

## Output

Return the messured time on stdout and verbose output on stderr.
