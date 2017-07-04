#!/bin/bash


#FILES=/data/Phd_final/Phd_evaluation/durative/domains/tms/POPMER_OVHPOP/config_
FILES=/data/Phd_final/Phd_evaluation/durative/domains/tms/POPMER_OVHPOP/
RESULT=/data/Phd_final/Phd_evaluation/durative/results/vhpop_comparison/POPMER_IPCVHPOP_30min_8GB/
size1=${#FILES}


./mymerge.bash $FILES $RESULT $size1

str="mymerge_"
type=5

FILES=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/co_origin/p21_M
domain=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl
end=${#FILES}
tolerance=0.01

./extractPlans.bash $RESULT

./validate.bash $FILES $domain $RESULT $end $tolerance $type

numoffile=23
./../c/parse_files $RESULT $RESULT$str $type $numoffile
