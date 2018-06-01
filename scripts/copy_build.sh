#! /usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh

cp $source_dir/distribute/target/scala-2.11/metabolite-distribute_2.11-1.0.jar $cluster/

while read name
do
    echo "buka"
    if [ "$name" = "$master" ]; then
        echo "Buka2 $name $master"
        continue
    fi
  echo "============================== syncing to:" $name "==================================="

  scp $cluster/metabolite_glpk $user@$name:$cluster
  scp $cluster/metabolite-distribute_2.11-1.0.jar $user@$name:$cluster
  scp $cluster/scopt_2.11-3.7.0.jar $user@$name:$cluster

done < $slave_file
