/**
  * Created by grzegorzbokota on 07.06.2017.
  */
import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf
import org.apache.spark.sql.SparkSession


class Grouping {
  def main(args: Array[String]): Unit = {
    if (args.length != 3){
      println(args(0).toString)
      sys.error("Wrong execution arguments\n")
      System.exit(1)
    }

    val conf = new SparkConf().setAppName("Grouping Application") //.setMaster("local[4]")
    val sc = new SparkContext(conf)
    val ss = SparkSession.builder().getOrCreate()
    import ss.implicits._
    val data = sc.textFile(args(1).toString)
    val key_data = data.map(v => (v.split("\\|")(1).trim, v))
    val df = key_data.toDF("target", "main")
    df.write.partitionBy("target").text(args(1))
  }

}
