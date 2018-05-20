#!/bin/bash

mpic++ --prefix /usr/local/share/OpenMPI -o mss mss.cpp

values=0
tries=300

for (( i=10; i <= 20; i++))
do
    let values=$((2**$i))
    echo "Testing for: $values"
    
    dd if=/dev/urandom bs=1 count=$values of=numbers >/dev/null 2>&1
    
    echo "Start logN:"
    
    for (( cnt=0 ; cnt < $tries; cnt++))
    do
        if [ $(( $cnt%25)) -eq 0 ]; then
            echo $cnt
        fi
        mpirun --prefix /usr/local/share/OpenMPI -np $i mss -time | grep 'Time'| cut -d ':' -f 2 >> $values"_logN".txt
    done
    echo "End logN"
    
    echo "Start 1 proc:"
    for (( cnt=0 ; cnt < $tries; cnt++))
    do
        if [ $(( $cnt%25)) -eq 0 ]; then
            echo $cnt
        fi
        mpirun --prefix /usr/local/share/OpenMPI -np 1 mss -time | grep 'Time'| cut -d ':' -f 2 >> $values"_1".txt
    done
    echo "End 1 proc"

    echo "Start 2 proc:"
    for (( cnt=0 ; cnt < $tries; cnt++))
    do
        if [ $(( $cnt%25)) -eq 0 ]; then
            echo $cnt
        fi
        mpirun --prefix /usr/local/share/OpenMPI -np 2 mss -time | grep 'Time'| cut -d ':' -f 2 >> $values"_2".txt
    done
    echo "End 2 proc"

    echo "Start 4 proc:"
    for (( cnt=0 ; cnt < $tries; cnt++))
    do
        if [ $(( $cnt%25)) -eq 0 ]; then
            echo $cnt
        fi
        mpirun --prefix /usr/local/share/OpenMPI -np 4 mss -time | grep 'Time'| cut -d ':' -f 2 >> $values"_4".txt
    done
    echo "End 4 proc"

    echo "Start 8 proc:"
    for (( cnt=0 ; cnt < $tries; cnt++))
    do
        if [ $(( $cnt%25)) -eq 0 ]; then
            echo $cnt
        fi
        mpirun --prefix /usr/local/share/OpenMPI -np 8 mss -time | grep 'Time'| cut -d ':' -f 2 >> $values"_8".txt
    done
    echo "End 8 proc"

    echo "Start 16 proc:"
    for (( cnt=0 ; cnt < $tries; cnt++))
    do
        if [ $(( $cnt%25)) -eq 0 ]; then
            echo $cnt
        fi
        mpirun --prefix /usr/local/share/OpenMPI -np 16 mss -time | grep 'Time'| cut -d ':' -f 2 >> $values"_16".txt
    done
    echo "End 16 proc"

done

rm -f mss numbers