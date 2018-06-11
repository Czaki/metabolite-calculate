#!/usr/bin/env bash

set -e

src=$1
dest=$2
columns=$3

for file in `ls $src`; do
 cat ${src}/${file} | cut -f $3 -d ' ' > ${dest}/${file}
done
