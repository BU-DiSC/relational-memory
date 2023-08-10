#!/bin/bash

ODIR=../objects

NUM_REVS=4
NUM_SAMPLES=10 #$1

MB=$((1024*1024))

RELCACHE=2
RELCACHE_SIZE=$((2*MB))

TARGET_SIZES=(2 4 8 16 32)

ROW_SIZE=512

#TOTAL_WIDTH=0

BENCH="q1_mvcc_col4"
VERSIONS="v4_multi_col"

ENABLED_COL_NUM=8
COL_OFF="0,32,72,140,260,340,396,480"
COL_WIDTHS="4,4,4,4,4,4,4,4"
COL_WIDTH=4

# calc the size of target data
WIDTHS=$(echo ${COL_WIDTHS} | sed "s/,/ /g")
TOTAL_WIDTH=0
for width in $WIDTHS
do
  TOTAL_WIDTH=$((TOTAL_WIDTH + width))
done

result="result_scale.csv"

for version in ${VERSIONS}
do
   bash update_relcache_version.sh $version > /dev/null &
   wait $!        	

  for bench in ${BENCH}
  do
    mv ${result} ${result}_old 2>/dev/null
    touch ${result}
    echo "bench, mem, temp, row_size, row_count, col_width, cycles, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired" >> ${result}

  
    for target in ${TARGET_SIZES[@]}
    do
      ratio=$((target/RELCACHE))
      BASE_ROW_COUNT=$((RELCACHE_SIZE/TOTAL_WIDTH))
      DBSIZE=$((ratio*ROW_SIZE*BASE_ROW_COUNT))
      ROW_COUNT=$((ratio*BASE_ROW_COUNT))
      echo "target: "$target "ratio: "$ratio "rowcnt: "$ROW_COUNT
  
      for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
      do
        #populate
        num_columns=$((${ROW_SIZE}/${COL_WIDTH}))
        width="${COL_WIDTH}"
        for ((num= 2 ; num<= $((num_columns)) ; num++)) 
        do
          width="${width},${COL_WIDTH}"
        done
        ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} #> /dev/null
        #echo "populate done."

        #execution query
        ${ODIR}/${bench} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${COL_WIDTHS} -O{COL_OFF} -S d >> ${result} 
        echo "DRAM query done"
  
  #####################
        touch temp.csv
        start=0
        while [ $start -lt ${DBSIZE} ]
        do
          ${ODIR}/db_reset_relcache 0
          ${ODIR}/db_reset_relcache 1
          ${ODIR}/db_reset_relcache 0
          ${ODIR}/db_reset_relcache 0  >/dev/null &
          wait $!

          rowcnt=${BASE_ROW_COUNT}
          dbsize=$((rowcnt*ROW_SIZE))
          end=$((start+dbsize))
          if (( ${end} > ${DBSIZE}-1 )); then
            end=$DBSIZE
          fi
          #echo $start $end
  
          ${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${rowcnt} -C ${ENABLED_COL_NUM} -W ${COL_WIDTHS} -O ${COL_OFF} -F ${start} > /dev/null
          wait $!
          #echo "config done."
  
          #execution query
          ${ODIR}/${bench} -r ${ROW_SIZE} -R ${rowcnt} -C ${ENABLED_COL_NUM} -W ${COL_WIDTHS} -O ${COL_OFF} -S p >> temp.csv &
          wait $!
          #echo "RelMem query done"
  
          start=$end
        done
        cat temp.csv | awk -v ROW_COUNT=${ROW_COUNT} -F',' '{sum+=$8;} END {print $1 ", "$2 ", "$3 ", "$4 ", "$5 ", "ROW_COUNT ", "$7 ", "sum}' >> ${result} 
        #cat temp.csv | awk -v ROW_COUNT=${ROW_COUNT} -F',' '{sum+=$7;l1_ref+=$8;l1_fill+=$9;l2_ref+=$10;l2_fill+=$11;inst+=$12} END {print $1 ", "$2 ", "$3 ", "$4 ", "ROW_COUNT ", "$6 ", "sum", "l1_ref", "l1_fill", "l2_ref", "l2_fill", "inst}' >> ${result} 
        rm temp.csv
      done
    done

    ################ Column store #####################

    for target in ${TARGET_SIZES[@]}
    do
      ratio=$((target/RELCACHE))
      BASE_ROW_COUNT=$((RELCACHE_SIZE/TOTAL_WIDTH))
      DBSIZE=$((ratio*ROW_SIZE*BASE_ROW_COUNT))
      ROW_COUNT=$((ratio*BASE_ROW_COUNT))
      echo "target: "$target "ratio: "$ratio "rowcnt: "$ROW_COUNT
  
      for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
      do
        #populate
        num_columns=$((${ROW_SIZE}/${COL_WIDTH}))
        width="${COL_WIDTH}"
        for ((num= 2 ; num<= $((num_columns)) ; num++)) 
        do
          width="${width},${COL_WIDTH}"
        done
        ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -S c #> /dev/null
        #echo "populate done."

        #execution query
        ${ODIR}/${bench} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${COL_WIDTHS} -O{COL_OFF} -S c >> ${result} 
        #echo "columnar query done"
      done
    done

  done
done

