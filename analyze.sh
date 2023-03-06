#!/bin/bash

if [ $# -eq 1 ]; then
    # Your code here. (1/4)
	grep -E "ERROR|WARN" $1 >> bug.txt
else
    case $2 in
    "--latest")
        # Your code here. (2/4)
		tail -5 $1
    ;;
    "--find")
        # Your code here. (3/4)
		grep -E $3 $1 > "${3}.txt"
    ;;
    "--diff")
        # Your code here. (4/4)
		if diff $1 $3 > /dev/null; then
			echo same
		else
			echo different
		fi		
    ;;
    esac
fi
