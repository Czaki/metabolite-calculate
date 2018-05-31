#! /usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh

rm -fr $home/hdfsdata
mkdir -p $home/hdfsdata
mkdir -p $home/hdfsdata/namenode
mkdir -p $home/hdfsdata/datanode


cd $home
set -u
master=$(head -n 1 $master_file)
echo $slave_file
ssh="ssh -i ~/.ssh/id_rsa"
scp="scp -i ~/.ssh/id_rsa"

ssh-add ~/.ssh/id_rsa
rm -f $home/cluster.tar
cd $home
tar -zcf $home/cluster.tar.gz cluster
while read name
do
    echo "buka"
    if [ "$name" = "$master" ]; then
        echo "Buka2 $name $master"
        continue
    fi
  echo "============================== syncing to:" $name "==================================="

  $ssh -n $user@$name rm -fr "$home/cluster $home/hdfsdata"
  $ssh -n $user@$name mkdir -p $home/hdfsdata
  $ssh -n $user@$name mkdir $home/hdfsdata/datanode
  scp $home/cluster.tar.gz $user@$name:$home
  $ssh -n $user@$name tar xf $home/cluster.tar.gz -C $home
  #$scp -r $home/cluster $user@$name:$home
done < $slave_file

hdfs namenode -format
