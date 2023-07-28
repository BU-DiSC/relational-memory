#!/bin/bash
ODIR=../objects

NUM_REVS=4
NUM_SAMPLES=10 #$1

NUM_COL_LIST=(4 8 16 32 64 128 256 512 1024 2048 4096)

min=0
max=100

for ((index = 0; index < ${#NUM_COL_LIST[@]}; index++))
do

ROW_COUNT=16384
row_size=${NUM_COL_LIST[$index]}

if [ $(($row_size == 4)) ]; then
	COL_OFF="0"
	ENABLED_COL_NUM=1
	K="0"
elif [ $(($row_size == 8)) ]; then
	COL_OFF="0"
	ENABLED_COL_NUM=1
	K="0"
else
	middle=$(((${NUM_COL_LIST[$index]} / 2) - 1))
	last=$((${NUM_COL_LIST[$index]} - 3))
	ENABLED_COL_NUM=9
	COL_OFF="0,$middle,$last"
	K="0,s,0,0,0,0,0,0,0"
fi

COL_WIDTHS="4"
FRAME_OFF=0x0
TYPE='c'

# K (g, s, 0)
# k (number)
# limit to 16k row
# " , ,  " depends on c (enabled column) value, s - smaller g - greater, 0 - just project
k=75

BENCH=q2_new
VERSIONS="v4_multi_col"

# mv CRAZY${BENCH}.csv CRAZY${BENCH}.csv_old 2>/dev/null
# touch CRAZY${BENCH}.csv

for version in ${VERSIONS}
do

    #echo "bench, mem, temp, row_size, row_count, col_width, cycles" >> CRAZY${BENCH}.csv
    for col_width in ${COL_WIDTHS}
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
	   	    ${ODIR}/db_generate -r ${row_size} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -m ${min} -M ${max} #> /dev/null
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
	   	    ${ODIR}/${BENCH} -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} -k ${k} -K ${K} >> CRAZY${BENCH}.csv #&
	   	    #wait $!
			echo "query done"
            
	        ${ODIR}/db_reset_relcache 0
	        ${ODIR}/db_reset_relcache 1
	        wait $!    		
        done
    done
	for col_width in ${COL_WIDTHS}
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
	   	    ${ODIR}/db_generate -r ${row_size} -R ${ROW_COUNT} -C ${num_columns} -W ${width} -S ${TYPE} -m ${min} -M ${max} #> /dev/null
			echo "populate done."

			#config
			width="${col_width}"
			for (( num= 1 ; num< $((ENABLED_COL_NUM)) ; num++ )) 
			do
				width="${width},${COL_WIDTH}"
			done
	   	    # ${ODIR}/db_config_relcache -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} #> /dev/null
			echo "config done."

	   	    #execution query
	   	    ${ODIR}/${BENCH} -r ${row_size} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} -k ${k} -K ${K} -S ${TYPE}>> CRAZY${BENCH}.csv # &
			#wait $!
			echo "query done"
            
	        ${ODIR}/db_reset_relcache 0
	        ${ODIR}/db_reset_relcache 1
	        wait $!    		
        done
    done
done

done
