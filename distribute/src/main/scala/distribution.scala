/* SimpleApp.scala */
import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf
import scala.io.Source

object SimpleApp {
  def main(args: Array[String]) {
    val logFile = "../data/HepatoNet1.PIPES.sfba" // Should be some file on your system
    val conf = new SparkConf().setAppName("Simple Application").setMaster("local[4]")
    val sc = new SparkContext(conf)
    val lines = Source.fromFile("../data/HepatocyteQSSPN.qsspn").getLines.toArray
    var i = 0
    var combination_num = 1

    while ( i < lines.length){
      val s = lines(i).toString.trim
      if (s.startsWith("ENZYME")){
        i += 1
        val s2 = lines(i).toString.trim
        combination_num *= s2.split(" ")(1).toInt
      }
      i += 1
    }
    println(combination_num)
    combination_num = 200
    val single_step = 4
    val ranges = {
      val base_range = List.range(0, combination_num, single_step)
      if (combination_num % single_step != 0)
        base_range ++ List(combination_num)
      else
        base_range
    }
    println(ranges zip ranges.tail)
    val distData = sc.makeRDD(ranges zip ranges.tail)
    val range_num = distData.count()
    val c = {
      distData.pipe("/Users/grzegorzbokota/Documents/projekty/metabolite-calculate/calculation/cmake-build-debug/metabolite ../data/HepatocyteQSSPN.qsspn ../data/HepatoNet1.PIPES.sfba -str")
    }
    print("Res: ")
    println(c.count())
    c.saveAsTextFile("../ex_res_txt")
    val logData = sc.textFile(logFile, 2).cache()
    val numAs = logData.filter(line => line.contains("a")).count()
    val numBs = logData.filter(line => line.contains("b")).count()
    println(s"Lines with a: $numAs, Lines with b: $numBs ranges: $range_num")
    sc.stop()
  }
}