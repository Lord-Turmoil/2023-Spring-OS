#!/bin/bash

mkdir -p mydir
chmod 777 mydir

echo 2023 > myfile

mv moveme mydir/moveme
cp copyme mydir/copied

cat readme

gcc bad.c -o bad 2> err.txt

dfl=10
n=${1:-10}
mkdir -p gen
cd gen
for ((i = 1; i <= n; i++)); do
	touch "${i}.txt"
done
cd ..

