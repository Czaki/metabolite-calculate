#!/usr/bin/env bash

script_dir="$(dirname "$0")"
. ${script_dir}/settings.sh

set -u

mkdir -p $download

if [ $# -eq 1 ]; then
    echo "ALA"
    tar -xvf $1
fi

if [ -d $cluster ]; then
    rm -rf $cluster
fi

mkdir -p $cluster


if [ "${custom_java}" = true ]; then
    if [ ! -e $download/jdk-8u171-linux-x64.tar.gz ]; then
        wget --no-check-certificate --no-cookies -O ${download}/jdk-8u171-linux-x64.tar.gz --header "Cookie: oraclelicense=accept-securebackup-cookie" "http://download.oracle.com/otn-pub/java/jdk/8u171-b11/512cd62ec5174c3487ac17c61aaa89e8/jdk-8u171-linux-x64.tar.gz"
    fi
    if [ ! -e $download/jdk-8u171-linux-x64.tar.gz ]; then
        echo "${RED}[ERROR] no java download${NC}"
        exit -1
    fi
fi

if [ ! -e $download/hadoop-${hadoop_version}.tar.gz ]; then
    wget -P ${download} www-eu.apache.org/dist/hadoop/common/hadoop-${hadoop_version}/hadoop-${hadoop_version}.tar.gz
fi
if [ ! -e $download/hadoop-${hadoop_version}.tar.gz ]; then
    echo "${RED}[ERROR] no hadoop download${NC}"
    exit -1
fi

if [ ! -e $download/spark-${spark_version}-bin-hadoop2.7.tgz ]; then
    wget -P ${download} http://ftp.man.poznan.pl/apache/spark/spark-${spark_version}/spark-${spark_version}-bin-hadoop2.7.tgz
fi
if [ ! -e $download/spark-${spark_version}-bin-hadoop2.7.tgz ]; then
    echo "${RED}[ERROR] no spark download${NC}"
    exit -1
fi

if [ ! -e $download/scala-${scala_version}.tgz ]; then
    wget -P ${home}/download downloads.lightbend.com/scala/${scala_version}/scala-${scala_version}.tgz
fi
if [ ! -e $download/scala-${scala_version}.tgz ]; then
    echo "${RED}[ERROR] no scala download${NC}"
    exit -1
fi

if [ ! -e $download/sbt-${sbt_version}.tgz ]; then
    wget -P ${home}/download https://piccolo.link/sbt-${sbt_version}.tgz
fi
if [ ! -e $download/sbt-${sbt_version}.tgz ]; then
    echo "${RED}[ERROR] no sbt download${NC}"
    exit -1
fi

tar -xvf $download/jdk-8u171-linux-x64.tar.gz -C $cluster
mv $cluster/jdk1.8.0_171 $jdk_path
tar -xvf $download/hadoop-${hadoop_version}.tar.gz -C $cluster
mv $cluster/hadoop-${hadoop_version} $hadoop_path
tar -xvf $download/scala-${scala_version}.tgz -C $cluster
mv $cluster/scala-${scala_version} $scala_path
tar -xvf $download/spark-${spark_version}-bin-hadoop2.7.tgz -C $cluster
mv $cluster/spark-${spark_version}-bin-hadoop2.7 $spark_path
tar -xvf $download/sbt-${sbt_version}.tgz -C $cluster
