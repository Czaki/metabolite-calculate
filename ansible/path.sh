script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${script_dir}/settings.sh

export PATH=$JAVA_HOME/bin:$SCALA_HOME/bin:$SBT_HOME/bin:$SPARK_HOME/bin:$SPARK_HOME/sbin:$HADOOP_INSTALL/bin:$HADOOP_INSTALL/sbin:$PATH
