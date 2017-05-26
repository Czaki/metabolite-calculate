script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source_dir=`dirname $script_dir`
home=$source_dir
download=${home}/download
cluster=${home}/cluster
build=${home}/build
RED='\033[0;31m'
NC='\033[0m' # No Color
jdk_path=$cluster/jdk
hadoop_path=$cluster/hadoop
scala_path=$cluster/scala
spark_path=$cluster/spark
stb_path=$cluster/sbt
master_file=$home/master
slave_file=$home/slaves 
user=$USERNAME

cpp_dir=$source_dir/calculation
