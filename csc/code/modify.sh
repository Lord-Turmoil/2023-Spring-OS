#!/bin/bash

input=$1
src=$2
dst=$3

pattern="s/${src}/${dst}/g"
sed -i ${pattern} ${input}
