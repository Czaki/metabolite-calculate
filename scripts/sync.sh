script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh

cd $home

while read name
do
  echo "============================== syncing to:" $name "==================================="
  
  rsync -zrvhae ssh $cluster $user@$name:$home
done < $slave_file 

