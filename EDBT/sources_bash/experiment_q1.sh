#!/bin/bash

ODIR=../objects

NUM_REVS=4
NUM_SAMPLES=1 #$1

ROW_SIZE=64
ROW_COUNT=32768
ENABLED_COL_NUM=3

# assume all columns have the identical width
# COL_WIDTH=1
COL_WIDTHS="1 2 4 8 16"
COL_OFF="0,24,48"

FRAME_OFF=0x0

BENCH=q1_col
VERSIONS="v4_multi_col"

for version in ${VERSIONS}
do
    mv PLT_result_${BENCH}.csv PLT_result_${BENCH}.csv_old 2>/dev/null
    touch PLT_result_${BENCH}.csv
    echo "bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired" >> PLT_result_${BENCH}.csv
    for COL_WIDTH in ${COL_WIDTHS}
      do
      for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
        do
    	    #bash update_relcache_version.sh $version #> /dev/null &
    	    wait $!
    	
            #populate
            num_columns=$((${ROW_SIZE}/${COL_WIDTH}))
            width="${COL_WIDTH}"
            for ((num= 2 ; num<= $((num_columns)) ; num++)) 
            do
              width="${width},${COL_WIDTH}"
            done
       	    ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} #> /dev/null
            echo "populate done."
	    	
       	    #config
            width="${COL_WIDTH}"
            for (( num= 1 ; num< $((ENABLED_COL_NUM)) ; num++ )) 
            do
              width="${width},${COL_WIDTH}"
            done
       	    ${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} #> /dev/null
            echo "config done."
	    	
       	    #execution query
       	    ${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} >> PLT_result_${BENCH}.csv &
       	    wait $!
            echo "query done"
            ${ODIR}/db_reset_relcache 0
            ${ODIR}/db_reset_relcache 1
            wait $!
        done
      done
done

