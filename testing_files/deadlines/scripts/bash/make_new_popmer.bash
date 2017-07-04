#!/bin/bash

FILES=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/POPMER_big_deadlines/config_*
RESULT=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/separate_problems_new_domain/
end=$((${#FILES}-1))

i=1
for f in $FILES
do
  str="0"$i
  echo $str"  "$end

  endname=${f:$end}

  if [ "$endname" == "$str" ]
  then
    echo $endname
    endname=$i
  fi

  endname="p21_"$endname".pddl"
  echo $endname
  ./../c/modify_popmer_files $f $RESULT$endname 0

  i=$(($i+1))
done
