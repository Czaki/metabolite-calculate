#! /usr/bin/env bash

script_dir="$(dirname "$0")"
. ${script_dir}/path.sh

begin=$(( $1*1000000 ))
end=$(( $1*1000000 ))
master=$(head -n 1 $master_file)

echo "a/"${master}"/b"
spark-submit --class Distribution --master spark://${master}.mimuw.edu.pl:6066 --deploy-mode cluster --jars ${cluster}/scopt_2.11-3.7.0.jar ${cluster}/metabolite-distribute_2.11-1.0.jar ${cluster}/metabolite_glpk ${cluster}/data part_result ${begin}=${end} -p 1000000 --single_step 10 --slices 10000
