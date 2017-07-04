#!/bin/bash

time[0]=2
time[1]=7
time[2]=15
time[3]=22
time[4]=26
time[5]=44
time[6]=50
time[7]=54
time[8]=72
time[9]=101
time[10]=118
time[11]=150
time[12]=153
time[13]=185
time[14]=212
time[15]=241
time[16]=271
time[17]=300
time[18]=337
time[19]=339
time[20]=350
time[21]=381
time[22]=404



FILES=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/co_origin/*
RESULT=/data/Phd_final/Phd_evaluation/durative/results/driverlog/dae_30min_6GB_4067/
i=0
#domain=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/domain_deadlines.pddl
domain=/data/Phd_final/Phd_evaluation/durative/domains/driverlog/domain.pddl

size=$((${#FILES}-1))

time_considered="false"

zero=0

value=1
if [ "$value" == "$zero" ]
then
  echo ";;;;" >> $RESULT"conti"
  echo " " >> $RESULT"memory"
  echo " " >> $RESULT"real_runtime"
fi

#origin="origin/"
origin=""
for p in $FILES
do
  echo ";Processing $p file..."
  echo ";"$i

  ulimit -t 1800
  ulimit -v 6000000 

  endname=${p:$size}
  endname=$(echo $endname| cut -d'.' -f 1)


  #if [ "$endname" -lt "$value" ]
  #then
  #  continue
  #fi

  output=$RESULT$origin$endname

  echo "this is output"
  echo $output

  
  file_index=1
  num_files="$(\ls $RESULT -afq | wc -l)"
  origin_files=$num_files

  START_TIME=0.0

  exec 3>&1 4>&2
  exec > >(tee $output)
  pidtee=$!
  exec 2>&1

  start=$(date +%s.%N)

  ./../../../../phd/Phd/temp_evaluation/source_code_ipc2014/tempo-sat-dae_yahsp/dae_yahsp --domain=$domain --instance=$p --plan-file=$output --seed=4067 &

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
      echo $endname";"$it";"$value";"$START_TIME";"$vmSize";" >> $RESULT"conti"
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

   if [ "$curr_files" != "$num_files" ]
   then
      num_files=$curr_files
      value=$(($num_files - $origin_files))
      it=$((i+1))
      echo $endname";"$it";"$value";"$START_TIME";"$vmSize";" >> $RESULT"conti"
      file_index=$((file_index+1))
   fi

   kill $pidtee
   exec 1>&3 2>&4
   echo ";done"
   echo $endname $vmSize $vmPeak  >> $RESULT"memory"
   echo $endname $runt >> $RESULT"real_runtime"

   
  i=$((i+1))
done

