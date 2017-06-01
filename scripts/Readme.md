# Scripts for setup hadoop and saprk cluster

## File description

1. `settings.sh` – script with variables used in others scripts. If in next points is used `$name` it is variable from this file.  
2. `download.sh` – script which download needed files to `$download` and unpack it to `$cluster`. It has one optional argument with is path to tar archive containing `$download`
3. `build.sh` - script which build C++ and Spark programs from this repo
4. `install.sh` – script which configure hadoop, spark and hdfs.
5. `copy.sh` – script which copy `$cluster` folder on all workers. Its deleted previous data.  
6. `start.sh` – script starting hadoop cluster
7. `stop.sh` – script stoping hadoop cluster
8. `sync.sh` – script which syncing `$cluster` from master to slaves
9. `master` - file which should contain information about master (one line). Can be changed by `$master_file` variable in `$settings.sh`
10. `slaves` – list of all workers. If master will be used as worker have to be named exactly that same.
11. `path.sh` – modify `$PATH` variable. Add path to all programs used to work with cluster. Useful to be sourced when working form commandline.

## First cluster setup

Order of call:
1. `download.sh`
2. `build.sh`
3. `install.sh`
4. `copy.sh`
5. `start.sh`
6. `source path.sh`
6. `hdfs dfs -mkdir -p /user/${user}`

## example run
``` bash
spark-submit --class SimpleApp --master spark://green01.mimuw.edu.pl:6066 --deploy-mode cluster cluster/metabolite-distribute_2.11-1.0.jar /tmp/metabolite-calculate/cluster/metabolite /tmp/metabolite-calculate/cluster/data one-target5 400000 500000
```
