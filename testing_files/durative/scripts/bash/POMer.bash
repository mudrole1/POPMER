#!/bin/bash


FILES=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/POPMER_OVHPOP/config_
RESULT=/data/Phd_final/Phd_evaluation/durative/results/vhpop_comparison/POPMER_IPCVHPOP_30min_8GB/
size1=${#FILES}


#./mymerge.bash $FILES $RESULT $size1

str="mymerge_"
type=0

FILES=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/co_origin/p21_M
domain=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl
end=${#FILES}
tolerance=0.01
./../../../deadlines/scripts/bash/validate.bash $FILES $domain $RESULT $end $tolerance 

./../../../deadlines/scripts/c/parse_files $RESULT $RESULT$str $type
