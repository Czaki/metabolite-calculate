#!/usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh

set -u



while read name
do
    ssh -n $user@$name "rm -rf ${home}"
done < ${slave_file}
