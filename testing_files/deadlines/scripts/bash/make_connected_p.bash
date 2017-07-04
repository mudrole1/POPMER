#!/bin/bash

FILES=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/POPMER_smaller_deadlines/config_*
RESULT=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/PDDL3_smaller_deadlines/
end=$((${#FILES}-1))

for f in $FILES
do
  endname="p21_M"${f:$end}
  echo $endname
  #last parameter 1 - yes, add time windows; -1 - generate original problems; 0 - generate problems in new domain without time windows
  #2 - PDDL 3 within
  ./../c/add_deadlines $f $RESULT$endname 2

done
