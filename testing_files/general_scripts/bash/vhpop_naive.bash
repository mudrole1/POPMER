#!/bin/bash

FILES=/data/Phd_final/Phd_evaluation/durative/domains/tms/orig_problems/tms-2-3-light-*
RESULT=/data/Phd_final/Phd_evaluation/durative/results/tms/vhpop_30min_6GB/
i=0
domain=/data/Phd_final/Phd_evaluation/durative/domains/tms/domain-itsat.pddl
time_considered="false"

size=$((${#FILES}-1))

zero=0

value=0
if [ "$value" == "$zero" ]
then
  echo ";;;;" >> $RESULT"conti"
  echo " " >> $RESULT"memory"
  echo " " >> $RESULT"real_runtime"
fi

for p in $FILES
do
  echo "Processing $f file..."

  ulimit -t 1800
  ulimit -v 6000000 

  endname=${p:$size}
  echo $endname 

  output=$RESULT$endname
  echo $output

  num_files="$(\ls $RESULT -afq | wc -l)"
  origin_files=$num_files
  START_TIME=0.0

  exec 3>&1 4>&2
  exec > >(tee $output)
  pidtee=$!
  exec 2>&1

  start=$(date +%s.%N)

  h='-h ADDR/ADDR_WORK/BUC/LIFO'
  if (test -z "`grep ':durative-actions' $domain`") then
    f='-f {n,s}LR/{l}MW_add -l 10000 -f {n,s}LR/{u}MW_add/{l}MW_add -l 100000 -f {n,s,l}LR -l 200000 -f {n,s,u}LR/{l}LR -l unlimited -v0'
  else
    f='-f {n,s}LR/{l}MW_add -l 12000 -f {n,s}LR/{u}MW_add/{l}MW_add -l 100000 -f {n,s,l}LR -l 240000 -f {n,s,u}LR/{l}LR -l unlimited -v0'
  fi

  
  #exec 
  #./../../../../../phd/Phd/temp_evaluation/vhpop-pure/vhpop/vhpop -g $h $f $domain $p &

   #original vhpop
  ./../../../../phd/Phd/temp_evaluation/vhpop-pure/vhpop/vhpop $domain $p &

  pid=$!

  vmSize="0"
  vmPeak="0"

 
while true
do
 if ! kill -s 0 $pid > /dev/null 2>&1; then
    echo ";Could not send SIGTERM to process $pid" >&2 
    break
 else
   res1=$(date +%s.%N)

   result=$(cat /proc/$pid/status | grep ^Vm) 
   emptystr=""
   if [ "$result" != "$emptystr" ]
   then
     vmPeak=$(echo $result | cut -f 2-3 -d" ")
     vmSize=$(echo $result | cut -f 5-6 -d" ") 
   fi

   curr_files="$(\ls $RESULT -afq | wc -l)"

   if [ "$curr_files" != "$num_files" ]
   then
      num_files=$curr_files
      value=$(($num_files - $origin_files))
      it=$((i+1))
      echo $it";"$value";"$START_TIME";"$vmSize";" >> $RESULT"conti"
      file_index=$((file_index+1))
   fi

   # do stuff in here

   res2=$(date +%s.%N)
   dt=$(echo "$res2 - $res1" | bc)
   difference=$(echo "0.1 - $dt" | bc)

   if [ $(bc <<< "$difference > 0") -eq 1 ]
   then
     sleep $difference
     dt=0.1
   fi

   START_TIME=$(echo $START_TIME + $dt | bc -l) 
   #echo ";time "$START_TIME
   
   t="true"
   if [ "$time_considered" == "$t" ]
   then
     if [ $(bc <<< "${time[$i]} <= $START_TIME") -eq 1 ]
     then
        kill $pid
        echo ";killed at"
        echo ";"$START_TIME">"${time[$i]}
        curr_files="$(\ls $RESULT -afq | wc -l)"
     
        break
     fi
   fi
fi
done

   end=$(date +%s.%N)
   runt=$( echo "$end - $start" | bc -l )
   wait $pid
   echo ";waited"

   curr_files="$(\ls $RESULT -afq | wc -l)" #added missing in optic bash

   if [ "$curr_files" != "$num_files" ]
   then
      num_files=$curr_files
      value=$(($num_files - $origin_files))
      it=$((i+1))
      echo $it";"$value";"$START_TIME";"$vmSize";" >> $RESULT"conti"
      file_index=$((file_index+1))
   fi

   kill $pidtee
   exec 1>&3 2>&4
   echo ";done"
   echo $vmSize $vmPeak >> $RESULT"memory"
   echo $runt >> $RESULT"real_runtime"

  echo "" >> $output
  echo "Whole time of vhpop running:"$ELAPSED_TIME  >> $output
  i=$((i+1))
done

