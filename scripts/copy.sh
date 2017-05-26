. ./setings.sh

rm -fr $home/hdfsdata
mkdir -p $home/hdfsdata
mkdir -p $home/hdfsdata/namenode
mkdir -p $home/hdfsdata/datanode


cd $home

master=$(head -n 1 $master_file)

while read name
do
    if [ $name -eq $master]; then
        continue
    fi
  echo "============================== syncing to:" $name "==================================="
  
  ssh -n $user@$name rm -fr "$home/cluster $home/hdfsdata"
  ssh -n $user@$name mkdir -p $home/hdfsdata
  ssh -n $user@$name mkdir $home/hdfsdata/datanode
  scp -r $home/cluster $user@$name:$home
done < $slave_file

