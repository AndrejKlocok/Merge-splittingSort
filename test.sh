#!/bin/bash
# Author:   Andrej Klocok, xkloco00
# Project:  Merge-splitting sort
# File:     test.sh

#compile
mpic++ --prefix /usr/local/share/OpenMPI -o mss mss.cpp

#number count
if [ $# -ne 2 ]; then
    echo "Usage: $0 number_count processors count"
    exit
fi;

numbers=$1
processors=$2

#create file with random numbers
dd if=/dev/urandom bs=1 count=$numbers of=numbers >/dev/null 2>&1

#get correct number of processors
if [ $processors -ge $numbers ]; then
    #p(n) < n
    let processors=numbers-1 
fi;

#run
mpirun --prefix /usr/local/share/OpenMPI -np $processors mss

#clean
rm -f mss numbers