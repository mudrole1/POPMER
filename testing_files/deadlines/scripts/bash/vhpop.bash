#!/bin/bash

time[0]=600
time[1]=600
time[2]=600
time[3]=600
time[4]=600
time[5]=600
time[6]=600
time[7]=600
time[8]=600
time[9]=600
time[10]=600
time[11]=600
time[12]=600
time[13]=600
time[14]=600
time[15]=600
time[16]=600
time[17]=600
time[18]=600
time[19]=600
time[20]=600
time[21]=600
time[22]=600
time[23]=600



FILES=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/connected_origin/p21_M*
RESULT=/data/Phd_final/Phd_evaluation/deadlines/results/origin/myversion_ipc3_voriginal/
i=0
#domain=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/domain_deadlines.pddl
domain=/data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/domain.pddl
size=$((${#FILES}-1))

time_considered="false"

zero=0

value=0
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
  ulimit -v 8000000 

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

  h='-h ADDR/ADDR_WORK/BUC/LIFO'
  if (test -z "`grep ':durative-actions' $domain`") then
    f='-f {n,s}LR/{l}MW_add -l 10000 -f {n,s}LR/{u}MW_add/{l}MW_add -l 100000 -f {n,s,l}LR -l 200000 -f {n,s,u}LR/{l}LR -l unlimited -v0'
  else
    f='-f {n,s}LR/{l}MW_add -l 12000 -f {n,s}LR/{u}MW_add/{l}MW_add -l 100000 -f {n,s,l}LR -l 240000 -f {n,s,u}LR/{l}LR -l unlimited -v0'
  fi
 
  #pure version
  ./../../../../../phd/Phd/temp_evaluation/vhpop-pure/vhpop/vhpop -g $h $f $domain $p &

  #./../../../../../phd/Phd/temp_evaluation/vhpop-pure/vhpop/vhpop $domain $p &

  #./../../../../POMer-private/vhpop/vhpop -g $h $f $domain $p &

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

   wait $pid
   echo ";waited"

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
   echo $START_TIME >> $RESULT"real_runtime"

   
  i=$((i+1))
done



