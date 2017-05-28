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

  def getFileExtension(file: File): String = {
    val name = file.getName
    try
      name.substring(name.lastIndexOf(".") + 1)
    catch {
      case e: Exception =>
        ""
    }
  }

  def main(args: Array[String]) {
    if (args.length < 4){
      println(args(0).toString)
      sys.error("Wrong execution arguments\n")
      System.exit(1)
    }

    val executable = args(0)
    val file_list = getListOfFiles(args(1))
    if (file_list.isEmpty){
      System.err.println("[Error] No accces to qsspn and sfba file")
      System.exit(-1)
    }
    val qsspn_file = file_list.filter(getFileExtension(_) == "qsspn").head
    val sfba_file = file_list.filter(getFileExtension(_) == "sfba").head
    val conf = new SparkConf().setAppName("Simple Application") //.setMaster("local[4]")
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
    if (combination_num == 0){
      System.err.println("[Error] wrong combination num")
      System.exit(-1)
    }
    println(combination_num)
    val single_step = 100
    val distData =  if (args.length == 4){
      val pos = Source.fromFile(args(3)).getLines().toArray.map(_.toInt)
      val pos2 = pos.map(_+1)
      val ranges = pos zip pos2
      sc.makeRDD(ranges, 300)
    } else {
      if (args.length == 5) {
        val ranges = {
          val max_num = args(4).toInt
          val base_range = List.range(args(3).toInt, max_num, single_step)
          if (max_num % single_step != 0)
            base_range ++ List(max_num)
          else
            base_range
        }
        //println(ranges zip ranges.tail)
        sc.makeRDD(ranges zip ranges.tail, 300)
      } else {
        val ranges = {
          val base_range = List.range(0, combination_num, single_step)
          if (combination_num % single_step != 0)
            base_range ++ List(combination_num)
          else
            base_range
        }
        //println(ranges zip ranges.tail)
        sc.makeRDD(ranges zip ranges.tail, 300)
      }
    }

    val c = {
      distData.pipe(executable + " " + qsspn_file + " " + sfba_file + " -str")
    }
    //print("Res: ")
    //println(c.count())
    c.saveAsTextFile(args(2))
    sc.stop()
  }
}
