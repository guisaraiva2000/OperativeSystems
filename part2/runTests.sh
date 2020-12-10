#!/bin/bash

# ShellScript, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20
# Guilherme Saraiva 93717, Sara Ferreira 93756

inputDir=$1
outputDir=$2
maxthreads=$3
numbuckets=$4

#check number of arguments
if [ "$#" -ne 4 ]; then
    echo "Error... runTests.sh must receive 4 arguments." >&2
    exit 1
fi

#checks for input path
if [ ! -d "${inputDir}" ]; then
    echo "Error... Input path doesn't exist." >&2
    exit 1
fi

#checks for output path
if [ ! -d "${outputDir}" ]; then
    echo "Error... Output path doesn't exist." >&2
    exit 1
fi

re='^[0-9]+$'

#checks if maxthreads is an integer
if ! [[ ${maxthreads} =~ ${re} ]]; then
   echo "Error... Maxthreads must be an integer." >&2
   exit 1
fi

#checks if numbuckets is an integer
if ! [[ ${numbuckets} =~ ${re} ]]; then
   echo "Error... Number of buckets must be an integer" >&2
   exit 1
fi

#compiles
make

#run tests
for input in "$inputDir"/*.txt
do
  #cuts file extension
  filename=$(basename -- "$input")
  filename="${filename%.*}"

  echo InputFile="${input}" NumThreads="1"
  nosyncfile="$filename-1"

  #executes tecnicofs-nosync and supress errors
  ./tecnicofs-nosync "$input" "$outputDir"/"$nosyncfile".txt 1 1 2> /dev/null

  #run tecnicofs-mutex until it gets to maximum number of threads
  for((i = 2; i <= maxthreads; i++))
  do
    echo InputFile="${input}" NumThreads="${i}"
    mutexfile="$filename-$i"

    #executes tecnicofs-mutex and supress errors
    ./tecnicofs-mutex "$input" "$outputDir"/"$mutexfile".txt "$i" "$numbuckets" 2> /dev/null
  done
done
