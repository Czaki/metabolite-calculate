home=/home/jsroka

user=jsroka
cd $home

while read name
do
  echo "============================== syncing to:" $name "==================================="
  
  rsync -zrvhae ssh $home/cluster $user@$name:
done < slaves

