#!/bin/bash

input=$1
output=$2
i=1
IFS=$'\n'
echo -n "" > $output	# clear or create file
while read line; do
	if (($i==8)); then
		echo $line >> $output
	elif (($i==32)); then
		echo $line >> $output
	elif (($i==128)); then
		echo $line >> $output
	elif (($i==512)); then
		echo $line >> $output
	elif (($i==1024)); then
		echo $line >> $output
		break;
	fi
	i=$(($i+1))
done < $input

