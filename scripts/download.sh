#!/usr/bin/env bash
. ./settings.sh


mkdir -p $download

if [ -e $cluster ]; then
    rm -rf $cluster
fi

mkdir -p $cluster

#wget -P ${home}/download --no-check-certificate --no-cookies --header "Cookie: oraclelicense=accept-securebackup-cookie" http://download.oracle.com/otn-pub/java/jdk/8u111-b14/jdk-8u111-linux-x64.tar.gzi


if [ ! -e $download/jdk-8u131-linux-x64.tar.gz ]; then
    wget -P $download --no-check-certificate --no-cookies --header "Cookie: oraclelicense=accept-securebackup-cookie" http://download.oracle.com/otn-pub/java/jdk/8u131-b11/d54c1d3a095b4ff2b6607d096fa80163/jdk-8u131-linux-x64.tar.gz
fi
if [! -e $download/jdk-8u131-linux-x64.tar.gz ]; then
    echo "${RED}[ERROR] no java download${NC}"
    exit -1
fi 

if [ ! -e $download/hadoop-2.7.3.tar.gz ]; then
    wget -P ${download} www-eu.apache.org/dist/hadoop/common/hadoop-2.7.3/hadoop-2.7.3.tar.gz
fi 
if [ ! -e $download/hadoop-2.7.3.tar.gz ]; then
    echo "${RED}[ERROR] no hadoop download${NC}"
    exit -1
fi 

if [ ! -e $download/scala-2.11.6.tgz ]; then 
    wget -P ${home}/download downloads.lightbend.com/scala/2.11.6/scala-2.11.6.tgz
fi
if [ ! -e $download/scala-2.11.6.tgz ]; then 
    echo "${RED}[ERROR] no scala download${NC}"
    exit -1
fi 

if [ ! -e $download/spark-2.0.2-bin-hadoop2.7.tgz ]; then 
    wget -P ${home}/download www-eu.apache.org/dist/spark/spark-2.0.2/spark-2.0.2-bin-hadoop2.7.tgz
fi
if [ ! -e $download/spark-2.0.2-bin-hadoop2.7.tgz ]; then 
    echo "${RED}[ERROR] no spark download${NC}"
    exit -1
fi

if [ ! -e $download/sbt-0.13.12.tgz ]; then
    wget -P ${home}/download https://dl.bintray.com/sbt/native-packages/sbt/0.13.12/sbt-0.13.12.tgz
fi
if [ ! -e $download/sbt-0.13.12.tgz ]; then
    echo "${RED}[ERROR] no sbt download${NC}"
    exit -1
fi

tar -xvf $download/jdk-8u131-linux-x64.tar.gz -C $cluster
mv $cluster/jdk1.8.0_131 $jdk_path
tar -xvf $download/hadoop-2.7.3.tar.gz -C $cluster
mv $cluster/hadoop-2.7.3 $hadoop_path
tar -xvf $download/scala-2.11.6.tgz -C $cluster
mv $cluster/scala-2.11.6 $scala_path
tar -xvf $download/spark-2.0.2-bin-hadoop2.7.tgz -C $cluster
mv $cluster/spark-2.0.2-bin-hadoop2.7 $spark_path
tar -xvf $download/sbt-0.13.12.tgz -C $cluster

