#!/bin/bash
FILESV=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/co_origin/p21_M
FILES=$FILESV"*"
domain=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl
end=$((${#FILES}-1))


RESULT=/data/Phd_final/Phd_evaluation/durative/results/driverlog/dae_30min_6GB_4067/
PREFIX="p21_M"
num=0.01

#need to cleen files

i=1
numoffiles=23
while true
do
  
  b=10
  if [ "$b" -gt "$i" ]
  then
   test=$RESULT$PREFIX"0"$i
  else
   test=$RESULT$PREFIX$i
  fi

  f=$test
  if [ -e "$f" ]
  then
    rm $f
  fi

  i=$((i+1))
  
  if [ "$i" -gt "$numoffiles" ]
  then
     break
  fi
done

./extractPlans.bash $RESULT$PREFIX 

type=5
./validate.bash $FILESV $domain $RESULT$PREFIX $end $num $type

echo "validated tfd"

type=5 #plans are now represented in my structure
./../c/parse_files $RESULT$PREFIX $RESULT $type
