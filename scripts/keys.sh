home=/home/jsroka

#sudo apt-get install ssh
#sudo apt-get install rsync
#chmod 0600 ~/.ssh/authorized_keys
#chmod 0700 ~/.ssh/

ssh-keygen -t rsa

user=jsroka
cd $home
master=$(head -n 1 $home/master)
ssh-copy-id $user@$master

while read name
do
  ssh-copy-id $user@$name
done < slaves

