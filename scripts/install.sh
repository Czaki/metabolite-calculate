script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh 

cd $cluster 

export JAVA_HOME=$jdk_path
export SCALA_HOME=$scala_path
export SBT_HOME=$stb_path
export HADOOP_INSTALL=$hadoop_path
export HADOOP_PREFIX=$HADOOP_INSTALL
export SPARK_HOME=$spark_path 

export PATH=$JAVA_HOME/bin:$SCALA_HOME/bin:$SBT_HOME/bin:$SPARK_HOME/bin:$SPARK_HOME/sbin:$HADOOP_INSTALL/bin:$HADOOP_INSTALL/sbin:$PATH

etc_hadoop=${HADOOP_INSTALL}/etc/hadoop
#hdfs_dir="$( mktemp -d /tmp/hadoop.XXXXXX )"
hdfs_dir="${home}/hdfsdata"
master=$(head -n 1 $master_file)
cp $script_dir/slaves $HADOOP_INSTALL/etc/hadoop/slaves



echo 
echo "***************************************************************************"
echo modifying ${HADOOP_INSTALL}/etc/hadoop/hadoop-env.sh
echo setting export JAVA_HOME=${JAVA_HOME}
echo "***************************************************************************"
sed -i -e "s|^export JAVA_HOME=\${JAVA_HOME}|export JAVA_HOME=$JAVA_HOME|g" ${HADOOP_INSTALL}/etc/hadoop/hadoop-env.sh

cat <<EOF > ${etc_hadoop}/core-site.xml
<configuration>
  <property>
    <name>fs.defaultFS</name>
    <value>hdfs://${master}:9000</value>
    <description>NameNode URI</description>
  </property>
</configuration>
EOF

cat <<EOF > ${etc_hadoop}/hdfs-site.xml
<configuration>
  <property>
    <name>dfs.replication</name>
    <value>3</value>
  </property>

  <property>
    <name>dfs.datanode.data.dir</name>
    <value>file://${hdfs_dir}/datanode</value>
    <description>Comma separated list of paths on the local filesystem of a DataNode where it should store its blocks.</description>
  </property>
 
  <property>
    <name>dfs.namenode.name.dir</name>
    <value>file://${hdfs_dir}/namenode</value>
    <description>Path on the local filesystem where the NameNode stores the namespace and transaction logs persistently.</description>
  </property>

  <property>
    <name>dfs.namenode.datanode.registration.ip-hostname-check</name>
    <value>false</value>
    <description>http://log.rowanto.com/why-datanode-is-denied-communication-with-namenode/</description>
  </property>
</configuration>
EOF


cat <<EOF > ${etc_hadoop}/mapred-site.xml
<configuration>
    <property>
        <name>mapreduce.framework.name</name>
        <value>yarn</value>
    </property>
</configuration>
EOF


cat <<EOF > ${etc_hadoop}/yarn-site.xml
<configuration>
    <property>
        <name>yarn.nodemanager.aux-services</name>
        <value>mapreduce_shuffle</value>
    </property>
    <property>
        <name>yarn.resourcemanager.hostname</name>
        <value>${master}</value>
   </property>
</configuration>
EOF

echo 
echo "***************************************************************************"
echo hdfs namenode -format
echo "***************************************************************************"
# Format the namenode directory (DO THIS ONLY ONCE, THE FIRST TIME)
hdfs namenode -format

echo 
echo "***************************************************************************"
echo configuring Spark
echo "***************************************************************************"

cat <<EOF > $SPARK_HOME/conf/spark-env.sh
#!/usr/bin/env bash

home=/tmp/hadoop
export JAVA_HOME=$JAVA_HOME
export SCALA_HOME=$SCALA_HOME
export SBT_HOME=$SBT_HOME
export HADOOP_INSTALL=$HADOOP_INSTALL
export HADOOP_PREFIX=$HADOOP_INSTALL
export SPARK_HOME=$SPARK_HOME

export SPARK_WORKER_CORES=`grep -c ^processor /proc/cpuinfo`     
export SPARK_PUBLIC_DNS=${master}
EOF

cp $slave_file $SPARK_HOME/conf/slaves 

