#!/bin/bash


FILESV=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/co_origin/p21_M
FILES=$FILESV"*"
#domain=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/domain_deadlines.pddl
domain=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl
end=$((${#FILES}-1))


RESULT=/data/Phd_final/Phd_evaluation/durative/results/driverlog/optic_30min_6GB/
PREFIX="p21_M"
str="optic_"
type=4

folder="origin/"
mkdir $RESULT"origin"
#to split files first
./../c/parse_files $RESULT$PREFIX $RESULT$str $type

echo "splitted files"
num=0.001

i=1
while true
do

  
  b=10
  if [ "$b" -gt "$i" ]
  then
   test=$RESULT$PREFIX"0"$i
   move=$RESULT$folder$PREFIX"0"$i
  else
   test=$RESULT$PREFIX$i
   move=$RESULT$folder$PREFIX$i
  fi


  if [ -e "$test" ]
  then
    mv $test $move
    i=$((i+1))
  else
    break
  fi
done

cp $RESULT"conti" $RESULT$folder"conti"
cp $RESULT"memory" $RESULT$folder"memory"
cp $RESULT"real_runtime" $RESULT$folder"real_runtime"

./extractPlans.bash $RESULT$PREFIX 

type=5
./validate.bash $FILESV $domain $RESULT$PREFIX $end $num $type

echo "validated optic"

type=5 #plans are now represented in my structure
./../c/parse_files $RESULT$PREFIX $RESULT $type

#echo "obtain files optic"
