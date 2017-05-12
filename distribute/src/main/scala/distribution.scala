/* SimpleApp.scala */
import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf
import scala.io.Source
import java.io.File



object SimpleApp {
  def getListOfFiles(dir: String):List[File] = {
    val d = new File(dir)
    if (d.exists && d.isDirectory) {
      d.listFiles.filter(_.isFile).toList
    } else {
      List[File]()
    }
  }

  def getFileExtension(file: File) = {
    val name = file.getName()
    try
      name.substring(name.lastIndexOf(".") + 1)
    catch {
      case e: Exception =>
        ""
    }
  }

  def main(args: Array[String]) {
    if (args.length < 2){
      println(args(0).toString)
      sys.error("Wrong execution arguments\n")
      System.exit(1)
    }
    val executalble = args(0)
    val file_list = getListOfFiles(args(1))
    val qsspn_file = file_list.filter(getFileExtension(_) == "qsspn").head
    val sfba_file = file_list.filter(getFileExtension(_) == "sfba").head
    val conf = new SparkConf().setAppName("Simple Application").setMaster("local[4]")
    val sc = new SparkContext(conf)
    val lines = Source.fromFile(qsspn_file).getLines.toArray
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
    combination_num = 20
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
      distData.pipe(executalble + " " + qsspn_file + " " + sfba_file + " -str")
    }
    print("Res: ")
    println(c.count())
    c.saveAsTextFile("../ex_res_txt")
    sc.stop()
  }
}