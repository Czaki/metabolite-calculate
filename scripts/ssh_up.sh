#! /usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh


echo -n "set password: "
read -s password
echo ""

while read name
do
  echo "=========================== set connection to:" $name "==============================="
  expect ssh_up.ex $name $password 
  echo ""

done < $slave_file

