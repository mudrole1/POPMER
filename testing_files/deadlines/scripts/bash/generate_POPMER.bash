#!/bin/bash


#FILES_OUT=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/POPMER_smaller_deadlines/
#FILES_IN=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/separate_problems/
#DOMAIN=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/domain.pddl

FILES_OUT=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/POPMER_OVHPOP/
FILES_IN=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/separate_problems/
DOMAIN=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl


MIN_MAKESPAN=$FILES_IN"min_makespan_new_domain"
end=$((${#FILES}-1))

#1 - big, 0 - midlle, -1 - impossible, 2 - smaller
value_deadline=1

#1 - sequential release, 2 random release
value_release=0
./../c/generate_problems $DOMAIN $FILES_IN $FILES_OUT $value_deadline $value_release $MIN_MAKESPAN

#for f in $FILES
#do
#  endname="p21_M"${f:$end}
#  echo $endname
  

#done
