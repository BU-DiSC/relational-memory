#!/bin/bash

ODIR=../objects

NUM_REVS=4
#NUM_SAMPLES=1 #$1
NUM_SAMPLES=30 #$1

MB=$((1024*1024))

RELCACHE=2
#RELCACHE_SIZE=2068000
RELCACHE_SIZE=$((2*MB))

#TARGET_SIZES="2"
TARGET_SIZES="2 4 8 16 32 64 128"

ROW_SIZE=200

TOTAL_WIDTH=0

BENCH="tpch_q1 tpch_q6"
VERSIONS="v4_multi_col"

for version in ${VERSIONS}
do
   bash update_relcache_version.sh $version #> /dev/null &
   wait $!        	

  for bench in ${BENCH}
  do
    mv PLT_result_${bench}.csv PLT_result_${bench}.csv_old 2>/dev/null
    touch PLT_result_${bench}.csv
    echo "bench, mem, temp, row_size, row_count, col_width, cycles, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired" >> PLT_result_${bench}.csv

    echo ${bench}
    if [[ ${bench} == "tpch_q1" ]]; then
      ENABLED_COL_NUM=7
      COL_OFF="32,40,48,56,64,65,79"
      COL_WIDTHS="8,8,8,8,1,1,13"
    elif [[ ${bench} == "tpch_q6" ]]; then
      echo "Q^"
      ENABLED_COL_NUM=4
      COL_OFF="32,40,48,79"
      COL_WIDTHS="8,8,8,13"
    fi
    WIDTHS=$(echo ${COL_WIDTHS} | sed "s/,/ /g")
    TOTAL_WIDTH=0
    for width in $WIDTHS
    do
      TOTAL_WIDTH=$((TOTAL_WIDTH + width))
    done
    echo "total width: "${TOTAL_WIDTH}

    for target in ${TARGET_SIZES}
    do
      ratio=$((target/RELCACHE))
      BASE_ROW_COUNT=$((RELCACHE_SIZE/TOTAL_WIDTH))
      DBSIZE=$((ratio*ROW_SIZE*BASE_ROW_COUNT))
      ROW_COUNT=$((ratio*BASE_ROW_COUNT))
      #echo $RELCACHE_SIZE $DBSIZE
      echo "ratio: "$ratio "rowcnt: "$ROW_COUNT
  
      #populate
      ${ODIR}/tpch -D ${DBSIZE} #> /dev/null
      echo "populate done."
  
      PARTITIONS=$(( `echo ${DBSIZE} ${RELCACHE_SIZE} | awk '{tmp=$1/$2 ; print tmp}' | cut -f1 -d"."`  + 1 ))
      #echo $PARTITIONS
  
      for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
      do
  
        #execution query
        #${ODIR}/${bench} -R ${ROW_COUNT} -S d 
        ${ODIR}/${bench} -R ${ROW_COUNT} -S d >> PLT_result_${bench}.csv &
        wait $!
        echo "DRAM query done"
  
  #####################
        touch temp.csv
        start=0
        while [ $start -lt ${DBSIZE} ]
        do
          ${ODIR}/db_reset_relcache 0
          ${ODIR}/db_reset_relcache 1
          ${ODIR}/db_reset_relcache 0
          ${ODIR}/db_reset_relcache 1
          ${ODIR}/db_reset_relcache 0
          wait $!    		
  	#echo $ROW_COUNT $start $DBSIZE
          rowcnt=${BASE_ROW_COUNT}
          dbsize=$((rowcnt*ROW_SIZE))
          end=$((start+dbsize))
          if (( ${end} > ${DBSIZE}-1 )); then
            end=$DBSIZE
          fi
  
          ${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${rowcnt} -C ${ENABLED_COL_NUM} -W ${COL_WIDTHS} -O ${COL_OFF} -F ${start} #> /dev/null
          #echo "config done."
  
          #execution query
          echo "${ODIR}/${bench} -R ${rowcnt} -F ${start} "
          #${ODIR}/${bench} -R ${rowcnt} -F ${start}
          ${ODIR}/${bench} -R ${rowcnt} -F ${start} >> temp.csv &
          wait $!
          echo "RelMem query done"
  
          start=$end
        done
        cat temp.csv | awk -v ROW_COUNT=${ROW_COUNT} -F',' '{sum+=$7;l1_ref+=$8;l1_fill+=$9;l2_ref+=$10;l2_fill+=$11;inst+=$12} END {print $1 ", "$2 ", "$3 ", "$4 ", "ROW_COUNT ", "$6 ", "sum", "l1_ref", "l1_fill", "l2_ref", "l2_fill", "inst}' >> PLT_result_${bench}.csv &
        rm temp.csv
      done
  
      ################ Column store #####################
  
      #populate
      ${ODIR}/tpch -D ${DBSIZE} -S c #> /dev/null
      echo "populate done."
  
      for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
      do
         	#execution query
         	${ODIR}/${bench} -R ${ROW_COUNT} -S c >> PLT_result_${bench}.csv &
         	#${ODIR}/${bench} -R ${ROW_COUNT} -S c
         	wait $!
          echo "columnar query done"
  
          ${ODIR}/db_reset_relcache 0
          ${ODIR}/db_reset_relcache 1
          wait $!    		
  
      done
    done
  done
done

