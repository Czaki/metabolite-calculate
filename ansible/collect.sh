#! /usr/bin/env bash

curent_num=$1
path=$2
result_dir=$3
step=200
elements_num=`ls ${path}/part* | wc -l`
echo $elements_num
begins=`seq 0 $step $(($elements_num-1))`
for i in $begins
do
  files=`seq -f "%05g" $i $(($i + step - 1)) | sed -e "s|^|${path}/part-|"`
  cat $files > ${result_dir}/part-`printf "%03d" $curent_num`m.txt
  curent_num=$(($curent_num+1))
done
