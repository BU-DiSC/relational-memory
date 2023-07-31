#!/bin/bash

ODIR=../2objects

NUM_REVS=4
NUM_SAMPLES=30 #$1

ROW_COUNT=43690
ENABLED_COL_NUM=2

COL_OFF="0,8"

COL_WIDTHS=(1 2 4)
TYPE='c'

FRAME_OFF=0x0

K=0x88

BENCH=q2_col
VERSIONS="v4_multi_col"

for version in ${VERSIONS}
do
    mv PLT2_result_${BENCH}.csv PLT2_result_${BENCH}.csv_old
    touch PLT2_result_${BENCH}.csv
    #echo "bench, mem, temp, row_size, row_count, col_width, cycles" >> PLT2_result_${BENCH}.csv
    for col_width in ${COL_WIDTHS}
    do
        for (( row_size=16; row_size<=1024; row_size*=2 ))
        do

            for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
            do
	       	    #bash update_relcache_version.sh $version #> /dev/null &
	       	    wait $!        	

	            #populate
	            num_columns=$((${row_size}/${col_width}))
				width="${col_width}"
				for ((num= 2 ; num<= $((num_columns)) ; num++)) 
				do
					width="${width},${col_width}"
				done
				#echo $width
				#echo "${ODIR}/db_generate -r ${row_size} -R ${ROW_COUNT} -C ${num_columns} -W ${width}"
	       	    ${ODIR}/db_generate -r ${row_size} -R ${ROW_COUNT} -C ${num_columns} -W ${width} #> /dev/null
				echo "populate done."

	       	    #config
				width="${col_width}"
				for (( num= 1 ; num< $((ENABLED_COL_NUM)) ; num++ )) 
				do
					width="${width},${col_width}"
				done
	       	    ${ODIR}/db_config_relcache -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} #> /dev/null
				echo "config done."

	       	    #execution query
	       	    ${ODIR}/${BENCH}${col_width} -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} ${K} >> PLT2_result_${BENCH}.csv &
	       	    wait $!
				echo "query done"

	            ${ODIR}/db_reset_relcache 0
	            ${ODIR}/db_reset_relcache 1
	            wait $!    		
		    exit
            done
        done
    done
	for col_width in ${COL_WIDTHS}
    do
        for (( row_size=16; row_size<=1024; row_size*=2 ))
        do

            for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
            do
	       	    #bash update_relcache_version.sh $version #> /dev/null &
	       	    wait $!        	

	            #populate
	            num_columns=$((${row_size}/${col_width}))
				width="${col_width}"
				for ((num= 2 ; num<= $((num_columns)) ; num++)) 
				do
					width="${width},${col_width}"
				done
				#echo $width
				#echo "${ODIR}/db_generate -r ${row_size} -R ${ROW_COUNT} -C ${num_columns} -W ${width}"
	       	    
				${ODIR}/db_generate -r ${row_size} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -S ${TYPE}#> /dev/null
				echo "populate done."

	       	    #config
				width="${col_width}"
				for (( num= 1 ; num< $((ENABLED_COL_NUM)) ; num++ )) 
				do
					width="${width},${col_width}"
				done
	       	    ${ODIR}/db_config_relcache -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} #> /dev/null
				echo "config done."

	       	    #execution query
	       	    ${ODIR}/${BENCH}${col_width} -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} ${K} -S ${TYPE}>> PLT2_result_${BENCH}.csv &
	       	    wait $!
				echo "query done"

	            ${ODIR}/db_reset_relcache 0
	            ${ODIR}/db_reset_relcache 1
	            wait $!    		
		    exit
            done
        done
    done
done