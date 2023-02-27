#!/bin/bash

input=$1
output=$2
IFS='\n'
echo "" > $output
while read line; do
	echo "\"${line}\"" >> $output
done < $input