#!/bin/bash

CONFIG=/data/Phd_final/Phd_evaluation/durative/domains/tms/POPMER_OVHPOP/
FILES=/data/Phd_final/Phd_evaluation/durative/domains/tms/orig_problems/tms-2-3-light-*
NEW_FILES=/data/Phd_final/Phd_evaluation/durative/domains/tms/
DOMAIN=/data/Phd_final/Phd_evaluation/durative/domains/tms/domain-changed.pddl
END=-f
END2=/
i=1
numfiles=0


for p in $FILES
do
  #echo "Processing $f file..."
  #output=$p$END$END2
  
  #outputt=$NEW_FILES$i$END2
  outputt=$NEW_FILES"temp"$END2
  
  echo $outputt
  if [ ! -d "$outputt" ]; then
    mkdir -m 777 $outputt
  fi
  # take action on each file. $f store current file name
  #save number of goals to numfiles
  numfiles="$(./../c/split_file $p $outputt )"

    b=10
    if [ "$b" -gt "$numfiles" ]
    then
      raw_path=$NEW_FILES"0"$numfiles    
    else
      raw_path=$NEW_FILES$numfiles    
    fi

    subfile=0
    output=$raw_path"_"$subfile
    
    while [ -d "$output" ]
    do
      subfile=$((subfile+1))
      output=$raw_path"_"$subfile
    done
    #mv  $outputt/* $output/
  

    flag="-r"
   cp $flag $outputt $output
   rm $flag $outputt

  #create a config

 

  echo $numfiles
  endnameA=$p$END
  endname=${endnameA:66} 
  echo $endname

  #FILENAME=$CONFIG$endname

  b=10
  if [ "$b" -gt "$numfiles" ]
    then
      FILENAME=$CONFIG"0"$numfiles"_"$subfile
    else
      FILENAME=$CONFIG$numfiles"_"$subfile
    fi

  echo "1;-v0;" >> $FILENAME

  for ((j=0; j<numfiles; j++)); 
  do
   echo "0;100000;"$DOMAIN";"$output$END2$j";" >> $FILENAME
  done

  i=$((i+1))
  echo $i
  
done



