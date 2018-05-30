#! /usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/path.sh

set -u

echo
echo "***************************************************************************"
echo stop-yarn.sh
echo "***************************************************************************"
stop-yarn.sh

echo
echo "***************************************************************************"
echo stop-dfs.sh
echo "***************************************************************************"
stop-dfs.sh

echo
echo "***************************************************************************"
echo stop-all.sh #for Spark
echo "***************************************************************************"
$SPARK_HOME/sbin/stop-all.sh
