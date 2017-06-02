#! /usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh

while read name
do
  echo "============================== chcecking:" $name "==================================="
  
  ping -c1 -W1 $name 
  res=$?
  if [ $res -eq 0 ]; then
      echo $name >> $slave_file
  else
      echo "host unrechable"
  fi
done < $1

