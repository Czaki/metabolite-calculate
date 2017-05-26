echo 
echo "***************************************************************************"
echo start-dfs.sh
echo "***************************************************************************"
start-dfs.sh

echo 
echo "***************************************************************************"
echo start-yarn.sh
echo "***************************************************************************"
start-yarn.sh

echo 
echo "***************************************************************************"
echo sleep 30
echo "***************************************************************************"
sleep 30

echo 
echo "***************************************************************************"
echo hdfs dfsadmin -report
echo "***************************************************************************"
hdfs dfsadmin -report


echo 
echo "***************************************************************************"
echo start-all.sh #for Spark
echo "***************************************************************************"

$SPARK_HOME/sbin/start-all.sh

