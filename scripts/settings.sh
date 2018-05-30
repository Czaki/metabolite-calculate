script_dir="$( cd "$( dirname "$0" )" && pwd )"
source_dir=`dirname $script_dir`
#home=$source_dir
home="/tmp/hadoop"
download=${home}/download
cluster=${home}/cluster
build=${home}/build
RED='\033[0;31m'
NC='\033[0m' # No Color
hadoop_path=$cluster/hadoop
spark_path=$cluster/spark
scala_path=$cluster/scala
sbt_path=$cluster/sbt
master_file=$script_dir/master
slave_file=$script_dir/slaves

user=`whoami`
export _JAVA_OPTIONS=""
cpp_dir=$source_dir/calculation
hdfs_dir="${home}/hdfsdata"
namenode_dir="${hdfs_dir}/namenode"
datanode_dir="${hdfs_dir}/datanode"
custom_java=true
hadoop_version="2.8.3"
spark_version="2.3.0"
scala_version="2.11.7"
sbt_version="1.1.6"

if [ "${custom_java}" == true ] ; then
    jdk_path=$cluster/jdk
else
    jdk_path='/usr/lib/jvm/java-8-openjdk-amd64'
fi

export JAVA_HOME=${jdk_path}
export HADOOP_INSTALL=${hadoop_path}
export SPARK_HOME=${spark_path}
export HADOP_PREFIX=${HADOOP_INSTALL}

export PATH=$JAVA_HOME/bin:$HADOOP_INSTALL/bin:$HADOOP_INSTALL/sbin:$SPARK_HOME/bin:$SPARK_HOME/sbin:$PATH
