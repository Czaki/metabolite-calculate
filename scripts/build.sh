script_dir="$(dirname "$0")"
. ${script_dir}/path.sh

set -u

if [ ! -d $cluster ]; then
    echo "${RED}${cluster} directory not exists${NC}"
    mkdir -p $cluster 
    #exit -1 
fi

if [ ! -d $build ] || [ ! -e $build/metabolite ]; then
    mkdir -p $build
    cd $build
    cmake -DCMAKE_BUILD_TYPE=Release $cpp_dir
    make -j 4
fi
if [ ! -e $build/metabolite ]; then 
    echo "${RED}Build fail${NC}"
fi
cp $build/metabolite $cluster/

echo $_JAVA_OPTIONS
cd $source_dir/distribute
sbt package 
cp target/scala-2.11/metabolite-distribute_2.11-1.0.jar $cluster/
cp -r $source_dir/data $cluster/
cp -r $source_dir/data-simple $cluster/
