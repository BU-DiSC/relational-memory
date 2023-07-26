#!/bin/bash

ODIR=../objects

NUM_REVS=4
NUM_SAMPLES=30 #$1

ROW_SIZE=64
ROW_COUNT=32768
ENABLED_COL_NUM=2

COL_WIDTHS="4"

COL_OFF="0,8"

FRAME_OFF=0x0

BENCH=q5_col
VERSIONS="v4_multi_col"

for version in ${VERSIONS}
do
    mv PLT2_result_${BENCH}.csv PLT2_result_${BENCH}.csv_old 2>/dev/null
    touch PLT2_result_${BENCH}.csv
    echo "bench, mem, row_size, row_count, col_width, cycles" >> PLT2_result_${BENCH}.csv
    for col_width in ${COL_WIDTHS}
    do
        for (( row_size=16; row_size<=256; row_size=row_size+4 ))
        do
            for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
            do
           	    #bash update_relcache_version.sh $version #> /dev/null &
           	    wait $!        	
    
                width="${col_width}"
    			for (( num= 1 ; num< $((ENABLED_COL_NUM)) ; num++ )) 
    			do
    				width="${width},${col_width}"
    			done
                ${ODIR}/${BENCH}${col_width} -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} >> PLT1_result_${BENCH}.csv &
                wait $!        
    
    	        ${ODIR}/db_reset_relcache 0
    	        ${ODIR}/db_reset_relcache 1
            done
        done
    done
done

