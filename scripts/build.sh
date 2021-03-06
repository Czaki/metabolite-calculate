#! /usr/bin/env bash
script_dir="$(dirname "$0")"
. ${script_dir}/path.sh

set -u

if [ ! -d $cluster ]; then
    echo "${RED}${cluster} directory not exists${NC}"
    mkdir -p $cluster
    #exit -1
fi

if [ ! -d $build ] || [ ! -e $build/metabolite_glpk ]; then
    mkdir -p $build
fi
cd $build
cmake -DCMAKE_BUILD_TYPE=Release $cpp_dir
make -j 4
if [ ! -e $build/metabolite_glpk ]; then
    echo "${RED}Build fail${NC}"
fi
cp $build/metabolite_glpk $cluster/

echo $_JAVA_OPTIONS
cd $source_dir/distribute
sbt package
cp target/scala-2.11/metabolite-distribute_2.11-1.0.jar $cluster/
cp -r $source_dir/data $cluster/
cp -r $source_dir/data-simple $cluster/

cp $HOME/.ivy2/cache/com.github.scopt/scopt_2.11/jars/scopt_2.11-3.7.0.jar $cluster
