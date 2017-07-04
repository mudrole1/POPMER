#!/bin/bash

star="*"
FILES1=$1$star
domain=$2
RESULT=$3
end1=$4
tolerance=$5
type="$6"



mymerge="0"

if [ "$type" == "$type" ]
then
  echo "huraa"
fi
 
str="_valid"
out=1
#echo out

for p in $FILES1
do
  #echo "Processing $p file..."

  echo "p:"$p
  echo "end:"$end1
  endname=${p:$end1}


    b=10
    if [ "$b" -gt "$out" ]
    then
      output=$RESULT"0"$out
    else
      output=$RESULT$out
    fi
  j=1
  while true
  do

    
    if [ "$type" == "$mymerge" ]
    then
      echo "output: "$output
      
      test=$output
      save=$test$str  
    else
      echo $output
      test=$output"."$j
      
      save=$test$str
    fi


    if [ -e "$test" ]
    then
      
      #echo "domain:"$domain
      #echo "problem:"$p
      #echo "plan:"$test
      #echo "tolerance:"$tolerance

      ./../c/VAL-4.2.09/validate -t $tolerance $domain $p $test >> $save
      if [ "$type" == "$mymerge" ]
      then
         break;
      else
        j=$((j+1))
      fi
    else
      break
    fi
  done

  out=$((out+1))
done


