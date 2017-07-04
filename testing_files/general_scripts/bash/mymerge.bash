#!/bin/bash

str="*"
FILES=$1$str
RESULT=$2
end=$3
i=0


#if [ "$FILES" != "$str" ]
#then
  echo "ahoj"
  echo ";;;;" >> $RESULT"conti"
  echo " " >> $RESULT"memory"
  echo " " >> $RESULT"real_runtime"

  for f in $FILES
do
  echo "Processing $f file..."

  #ulimit -t 1800
  ulimit -v 8000000 

  echo "end:"$end
  endname=${f:$end}
  echo "endname:"$endname

  output=$RESULT$endname".1"
  echo $output

  START_TIME=0.0

  exec 3>&1 4>&2
  exec > >(tee $output)

  pidtee=$!
  exec 2>&1

  start=$(date +%s.%N)

  vhpop="./../../../../phd/Phd/temp_evaluation/vhpop-run/vhpop/vhpop"
  ./../../../POPMER-build-Debug/Merging $f $vhpop &
  #./Merging $f &

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

	   res2=$(date +%s.%N)
	   dt=$(echo "$res2 - $res1" | bc)
	   difference=$(echo "0.1 - $dt" | bc)

	   if [ $(bc <<< "$difference > 0") -eq 1 ]
	   then
	     sleep $difference
	     dt=0.1
	   fi

	   START_TIME=$(echo $START_TIME + $dt | bc -l) 
   fi

  done

   kill $pidtee

   exec 1>&3 2>&4

   echo ";done"
   endt=$(date +%s.%N)
   runt=$( echo "$endt - $start" | bc -l )

   echo $vmPeak $vmSize
   echo $endname $vmSize $vmPeak >> $RESULT"memory"
   echo $endname $runt >> $RESULT"real_runtime"

   
  i=$((i+1))
done
#fi





