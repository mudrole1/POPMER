#!/bin/bash


#FILES=/home/lenka/PostDoc/POPMER/testing_files/durative/domains/driverlog/POPMER/
#FILES=/home/lenka/PostDoc/POPMER/testing_files/deadlines/domains/driverlog/POPMER_impossible_deadlines/
FILES=/home/lenka/PostDoc/POPMER/testing_files/durative/domains/driverlog/POPMER_OVHPOP/
RESULT=/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/POPMER_0VHPOP_30min_6GB/
size1=${#FILES}

for i in `seq 1 10`;
do
  RESULTA=$RESULT$i
  mkdir $RESULTA
  RESULTA=$RESULTA"/"

  ./mymerge.bash $FILES $RESULTA $size1

  str="mymerge_"
  type=5

  FILES2=/home/lenka/PostDoc/POPMER/testing_files/deadlines/domains/driverlog/co_origin/p21_M
  domain=/home/lenka/PostDoc/POPMER/testing_files/deadlines/domains/driverlog/domain.pddl
  end=${#FILES2}
  tolerance=0.01

  ./extractPlans.bash $RESULTA

  RESULTA=$RESULTA"config_"
  ./validate.bash $FILES2 $domain $RESULTA $end $tolerance $type

  numoffile=23
  ./../c/parse_files $RESULTA $RESULTA$str $type $numoffile

done
