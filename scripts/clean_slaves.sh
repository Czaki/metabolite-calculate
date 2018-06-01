#!/usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh

set -u
echo ${slave_file}

ssh-add ~/.ssh/id_rsa

while read name
do
    echo "clean ${name}"
    ssh -n $user@$name "rm -rf ${home}"
    ssh -n $user@$name "rm -rf /tmp/*gb305058*"
done < ${slave_file}
echo "clean done"
