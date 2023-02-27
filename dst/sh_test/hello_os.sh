#!/bin/bash

input=$1
output=$2
i=0
IFS=$'\n'
while read line; do
	i=$(($i+1))
	if (($i==8)); then
		echo $line > $output
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
done < $input

