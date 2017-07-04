#!/bin/bash

star="*"
cesta=/data/Phd_final/Phd_evaluation/durative/results/vhpop_comparison/o_vhpop_sequence_30min_8GB/p21_s
#FILES=$1$star
FILES=$cesta$star

size=$((${#FILES}-1))

numoffiles="22"
b=10

for p in $FILES
do
  endname=${p:$size}
  endname=$(echo $endname| cut -d'.' -f 1)
  suffix=".1"

  numname=$(echo $endname| cut -d'0' -f 2)
  if [ "$numname" = """" ]
  then
    numname=$endname
  fi
  
  newfile=$(($numname + 1))
  if [ "$newfile" -lt "$b" ]
  then
    newfile="0"$newfile
  fi

  #echo $cesta$endname$suffix
  #echo $cesta$newfile$suffix

  ./../c/concatenate $cesta$endname$suffix $cesta$newfile$suffix

  rm $cesta$newfile$suffix
  mv $cesta$newfile$suffix"t" $cesta$newfile$suffix

  if [ "$endname" = "$numoffiles" ]
  then
    break
  fi
done
