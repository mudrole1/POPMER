#!/bin/bash

star="*"
FILES=$1$star

size=$((${#FILES}-1))

for p in $FILES
do

  conti='conti'
  memory='memory'
  rtime='real_runtime'

  if [[ $p == *$conti* ]]; then
    continue
  fi
  if [[ $p == *$memory* ]]; then
    continue
  fi
  if [[ $p == *$rtime* ]]; then
    continue
  fi

  ./../c/extractplan $p

  suffix=$(echo $p| cut -d'.' -f 2)

  if [ "$suffix" = "$p" ]
  then
     newfile=$p".1"
  else
     newfile=$p
  fi

  rm $p
  mv $newfile"t" $newfile
  


done
