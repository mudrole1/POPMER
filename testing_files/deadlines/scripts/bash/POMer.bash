#!/bin/bash


FILES=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/POPMER_impossible_deadlines/config_
RESULT=/data/Phd_final/Phd_evaluation/deadlines/results/impossible_deadlines/POPMER_30min_8GB/
size1=${#FILES}


#./mymerge.bash $FILES $RESULT $size1

str="mymerge_"
type=0

#this is wrong! type is not passed to validate
FILES=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/co_origin/p21_M
domain=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/domain.pddl
end=${#FILES}
tolerance=0.01
./validate.bash $FILES $domain $RESULT $end $tolerance 

./../c/parse_files $RESULT $RESULT$str $type
