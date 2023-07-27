#!/bin/bash

ODIR=../objects

NUM_REVS=4
NUM_SAMPLES=5 #$1


# 64 * 1024
# NUM_ROW_LIST=(16384 8192 4096 2048 1024 512 256 128 64 32 16 8 4)
# NUM_COL_LIST=(4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384)

NUM_ROW_LIST=(524288 262144 131072 65536 32768 16384 8192 4096 2048 1024 512 256 128 64 32 16 8 4)
NUM_COL_LIST=(4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288)

for ((index = 0; index < ${#NUM_COL_LIST[@]}; index++))
do    

ROW_SIZE=${NUM_COL_LIST[$index]}
ROW_COUNT=${NUM_ROW_LIST[$index]}
ENABLED_COL_NUM=1

#assume all columns have the identical width
COL_WIDTH=1
COL_OFF="0"

FRAME_OFF=0x0

BENCH=q1_col
VERSIONS="v4_multi_col"

for version in ${VERSIONS}
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
               ${ODIR}/${BENCH}${COL_WIDTH} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} >> bestthree.csv
               wait $!
            echo "query done"

            ${ODIR}/db_reset_relcache 0
            ${ODIR}/db_reset_relcache 1
            wait $!
        done
done
done