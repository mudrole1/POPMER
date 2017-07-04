#!/bin/bash

type=5

FILES=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/co_origin/p21_M
domain=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl
RESULT=/data/Phd_final/Phd_evaluation/durative/results/vhpop_comparison/ipc3_vhpop_sequence_30min_8GB/
PREFIX=p21_s
end=${#FILES}
tolerance=0.01

#./extractPlans.bash $RESULT$PREFIX 

./validate.bash $FILES $domain $RESULT$PREFIX $end $tolerance $type

./../c/parse_files $RESULT$PREFIX $RESULT $type 
