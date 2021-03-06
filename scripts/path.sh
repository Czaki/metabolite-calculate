script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${script_dir}/settings.sh

export JAVA_HOME=$jdk_path 
export SCALA_HOME=$scala_path 
export SBT_HOME=$sbt_path 
export HADOOP_INSTALL=$hadoop_path 
export HADOOP_PREFIX=$HADOOP_INSTALL 
export SPARK_HOME=$spark_path

export PATH=$JAVA_HOME/bin:$SCALA_HOME/bin:$SBT_HOME/bin:$SPARK_HOME/bin:$SPARK_HOME/sbin:$HADOOP_INSTALL/bin:$HADOOP_INSTALL/sbin:$PATH

