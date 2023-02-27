#!/bin/bash
#First you can use grep (-n) to find the number of lines of string.
#Then you can use awk to separate the answer.

input=$1
str=$2
output=$3
i=1

echo -n "" > $output	# create or clear file
while read line; do
	if [[ $line == *${str}* ]]; then
		# echo ${line}
		if (($i==1)); then
			echo $i >> $output
		else
			echo $i >> $output
		fi
	fi
	i=$(($i+1))
done < $input