#!/bin/bash

ODIR=../objects

NUM_REVS=4
#NUM_SAMPLES=1 #$1
NUM_SAMPLES=30 #$1

ROW_SIZE=64
ROW_COUNT=32768
ENABLED_COL_NUM="1 2 3 4 5 6 7 8 9 10 11"

# assume all columns have the identical width
COL_WIDTH=4
COL_OFF=4

FRAME_OFF=0x0

BENCH=q1_col
EXP_NAME=projectivity
VERSIONS="v4_multi_col"

for version in ${VERSIONS}
do
    mv result_${EXP_NAME}.csv result_${EXP_NAME}.csv_old 2>/dev/null
    touch result_${EXP_NAME}.csv
    for enabled_col_num in ${ENABLED_COL_NUM}
    do
	# without MVCC
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
            ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -T s #> /dev/null
            #echo "${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width}"
            echo "populate done."
	    	
       	    #config
            width="${COL_WIDTH}"
            offset=0
            col_offset="${offset}"
            for (( num= 1 ; num< $((enabled_col_num)) ; num++ )) 
            do
                width="${width},${COL_WIDTH}"
                offset=$((offset+COL_OFF))
                col_offset="${col_offset},${offset}"
            done
       	    ${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} #> /dev/null
       	    #echo "${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} #> /dev/null"
            echo "config done."
	    	
       	    #execution query
       	    ${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} | awk  -v var="$enabled_col_num"  '{print $1 $2 $3 $4 var", " $5 $6 $7 $8}' >> result_${EXP_NAME}.csv
       	    wait $!
            echo "query done"

            ${ODIR}/db_reset_relcache 0
            ${ODIR}/db_reset_relcache 1
            wait $!

    	    ################ Column store #####################
    	
            #populate
            num_columns=$((${ROW_SIZE}/${COL_WIDTH}))
            width="${COL_WIDTH}"
            for ((num= 2 ; num<= $((num_columns)) ; num++)) 
            do
                width="${width},${COL_WIDTH}"
            done
       	    ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -T s -S c #> /dev/null
            #echo "${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width}"
            echo "populate done."
	    	
       	    #config
            width="${COL_WIDTH}"
            offset=0
            col_offset="${offset}"
            for (( num= 1 ; num< $((enabled_col_num)) ; num++ )) 
            do
                width="${width},${COL_WIDTH}"
                offset=$((offset+COL_OFF))
                col_offset="${col_offset},${offset}"
            done
	    	
       	    #execution query
       	    ${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} -S c | awk  -v var="$enabled_col_num"  '{print $1 $2 $3 $4 var", " $5 $6 $7 $8}' >> result_${EXP_NAME}.csv
       	    #echo "${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} "
       	    wait $!
            echo "query done"

            ${ODIR}/db_reset_relcache 0
            ${ODIR}/db_reset_relcache 1
            wait $!
        done

        # with MVCC
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
       	    ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -T s -V #> /dev/null
            #echo "${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width}"
            echo "populate done."
	    	
       	    #config
            width="${COL_WIDTH}"
            offset=0
            col_offset="${offset}"
            for (( num= 1 ; num< $((enabled_col_num)) ; num++ )) 
            do
                width="${width},${COL_WIDTH}"
                offset=$((offset+COL_OFF))
                col_offset="${col_offset},${offset}"
            done
       	    ${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} -V #> /dev/null
       	    #echo "${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} #> /dev/null"
            echo "config done."
	    	
       	    #execution query
       	    ${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} -V | awk  -v var="$enabled_col_num"  '{print $1 $2 $3 $4 var", " $5 $6 $7 $8}' >> result_${EXP_NAME}.csv
       	    #echo "${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} -V"
       	    wait $!
            echo "query done"

            ${ODIR}/db_reset_relcache 0
            ${ODIR}/db_reset_relcache 1
            wait $!

    	    ################ Column store #####################
    	
            #populate
            num_columns=$((${ROW_SIZE}/${COL_WIDTH}))
            width="${COL_WIDTH}"
            for ((num= 2 ; num<= $((num_columns)) ; num++)) 
            do
                width="${width},${COL_WIDTH}"
            done
       	    ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -T s -S c -V #> /dev/null
            #echo "${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width}"
            echo "populate done."
	    	
       	    #config
            width="${COL_WIDTH}"
            offset=0
            col_offset="${offset}"
            for (( num= 1 ; num< $((enabled_col_num)) ; num++ )) 
            do
                width="${width},${COL_WIDTH}"
                offset=$((offset+COL_OFF))
                col_offset="${col_offset},${offset}"
            done
	    	
       	    #execution query
       	    ${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} -S c -V | awk  -v var="$enabled_col_num"  '{print $1 $2 $3 $4 var", " $5 $6 $7 $8}' >> result_${EXP_NAME}.csv
       	    echo "${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${enabled_col_num} -W ${width} -O ${col_offset} -F ${FRAME_OFF} -V"
       	    wait $!
            echo "query done"

            ${ODIR}/db_reset_relcache 0
            ${ODIR}/db_reset_relcache 1
            wait $!
        done
	done
done

