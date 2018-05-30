/**
  * Created by grzegorzbokota on 07.06.2017.
  */

import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf
import org.apache.spark.sql.SparkSession
import java.io.{File, PrintWriter}


object Grouping {
  def getListOfSubDirectories(directoryName: String): Array[String] = {
    new File(directoryName)
      .listFiles
      .filter(_.isDirectory)
      .map(_.getPath)
  }

  def main(args: Array[String]) {
    if (args.length != 2){
      println(args(0).toString)
      sys.error("Wrong execution arguments\n")
      System.exit(1)
    }

    val conf = new SparkConf().setAppName("Grouping Application") //.setMaster("local[4]")
    val sc = new SparkContext(conf)
    val ss = SparkSession.builder().getOrCreate()
    import ss.implicits._
    val data = sc.textFile(args(0).toString)
    val key_data = data.map(v => (v.split("\\|")(1).trim, v))
    val df = key_data.toDF("target", "main")
    df.write.partitionBy("target").text(args(1))
    val dirs = getListOfSubDirectories(args(1))
    val summary_file = new PrintWriter(new File(args(1), "summary.txt").toString)
    for (name <- dirs){
      val path = new File(name, "part*").toString
      val target_data = sc.textFile(path)
      val size = target_data.count()
      summary_file.write(name)
      summary_file.write(": ")
      summary_file.write(size.toString)
      summary_file.write("\n")
    }
    summary_file.close()
  }

}
