#!/bin/bash
#FILESV=/data/Phd_final/Phd_evaluation/durative/domains/tms/orig_problems/tms-2-3-light-
FILESV=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/co-origin/p21_M
FILES=$FILESV"*"
#domain=/data/Phd_final/Phd_evaluation/durative/domains/tms/domain-itsat.pddl
domain=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl
end=$((${#FILES}-1))


#RESULT=/data/Phd_final/Phd_evaluation/durative/results/tms/itsat_30min_6GB/
RESULT=/data/Phd_final/Phd_evaluation/durative/results/driverlog/itsat_30min_6GB/
PREFIX="p21_M"
#PREFIX=""
str="itsat_"
type=3
num=0.01

#need to cleen files
suffix1="-orders.txt"
suffix2="x"

i=1
numoffiles=23
#numoffiles=101
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

  f=$test$suffix1
  if [ -e "$f" ]
  then
    rm $f
  fi

  f=$test$suffix2
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

echo "validated itsat"

type=5 #plans are now represented in my structure
./../c/parse_files $RESULT$PREFIX $RESULT $type
