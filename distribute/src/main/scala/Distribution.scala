/* Distribution.scala */
import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf

import scala.io.Source
import java.io.File
import java.nio.file.{Files, Paths}

import sys.process._


case class Config (
                  executable: File = new File("metabolite_glpk"),
                  range: (Int, Int) = (0, 0),
                  metabolite_path: File = new File(""),
                  output_path: File = new File("output"),
                  parts_size: Int = 0,
                  sfba_file: File = new File(""),
                  qsspn_file: File =  new File(""),
                  target: String = "",
                  calculate_all: Boolean = true,
                  from_file: Boolean = false,
                  range_file: File = new File(""),
                  single_step: Int = 100,
                  slices : Int = 2000,
                  auto_copy: Boolean = false,
                  copy_path: File = new File(sys.env("HOME"), "tmp")
                  )


object Distribution {
  def getListOfFiles(dir: String): List[File] = {
    val d = new File(dir)
    if (d.exists && d.isDirectory) {
      d.listFiles.filter(_.isFile).toList
    } else {
      List[File]()
    }
  }

  def getListOfFiles(dir: File): List[File] = {
    if (dir.exists && dir.isDirectory) {
      dir.listFiles.filter(_.isFile).toList
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

  def get_parser(): scopt.OptionParser[Config] = {
    new scopt.OptionParser[Config]("distribute metabolite") {
      head("distribute metabolite", "1.0")
      arg[File]("executable").required().action((p, c) => c.copy(executable = p))
        .text("path to executable which calculate metabolite result")
      arg[File]("metabolite_path").required().action((p, c) => {
        val file_list = getListOfFiles(p)
        if (file_list.isEmpty) {
          System.err.println("[Error] No accces to qsspn and sfba file")
          System.exit(-1)
        }
        val qsspn_file = file_list.filter(getFileExtension(_) == "qsspn").head
        val sfba_file = file_list.filter(getFileExtension(_) == "sfba").head
        c.copy(metabolite_path = p, qsspn_file = qsspn_file, sfba_file = sfba_file)
      }).text("path to directory with qsspn and sfba files")
      arg[File]("output_path").required().action((p, c) => c.copy(output_path = p)).
        text("path to storage output")
      arg[(Int, Int)]("range").optional().action((p, c) => c.copy(range = p, from_file = false, calculate_all = false))
      opt[Int]('p', "parts_size").action((p, c) => c.copy(parts_size = p))
      opt[File]("range_file").action((p, c) => c.copy(range_file = p, from_file = true, calculate_all = false))
      opt[Int]("single_step").action((p, c) => c.copy(single_step = p)).text("number of marking in one task")
      opt[Int]("slices").action((p,c) => c.copy(slices = p)).text("number of slices in execution")
      opt[Unit]("with_backup").action((_, c) => c.copy(auto_copy = true))
      opt[File]("backup_path").action((p, c) => c.copy(copy_path = p))
    }
  }

  def run_calculation(sc: SparkContext, config: Config, range: (Int, Int), destination: String): Unit = {
    val distData = if (config.from_file) {
      val pos = Source.fromFile(config.range_file).getLines().toArray.map(_.toInt)
      val pos2 = pos.map(_ + 1)
      val ranges = pos zip pos2
      sc.makeRDD(ranges, config.slices)
    } else {
      val ranges =  List.range(range._1, range._2, config.single_step) ++ List(range._2)
      sc.makeRDD(ranges zip ranges.tail, config.slices)
    }

    val c = {
      distData.pipe(config.executable + " " + config.qsspn_file + " " + config.sfba_file + " --interactive" + {
        if (config.target != "") {
          "--target " + config.target
        } else {
          ""
        }
      })
    }
    c.saveAsTextFile(destination)
  }

  def calculate_number_of_combinations(config: Config): Int = {
    val lines = Source.fromFile(config.qsspn_file).getLines.toArray
    var i = 0
    var combination_num = 1

    while (i < lines.length) {
      val s = lines(i).toString.trim
      if (s.startsWith("ENZYME")) {
        i += 1
        val s2 = lines(i).toString.trim
        combination_num *= s2.split(" ")(1).toInt
      }
      i += 1
    }
    if (combination_num == 0) {
      System.err.println("[Error] wrong combination num")
      System.exit(-1)
    }
    combination_num
  }

  def main(args: Array[String]) {
    println(sys.env("HOME"))
    println(sys.env("SPARK_HOME"))
    val hdfs_executable = new File(new File(sys.env("HADOOP_INSTALL"), "bin"), "hdfs")
    val hdfs_get = hdfs_executable.toString + " dfs -get "
    var temp_dir = Paths.get("")
    val parser = get_parser()
    parser.parse(args, Config()) match {
      case Some(config) =>
        if (config.auto_copy) {
          temp_dir = Files.createTempDirectory("metabolite-output")

          if (! config.copy_path.exists()){
            Files.createDirectory(config.copy_path.toPath)
          }
        }
        val conf = new SparkConf().setAppName("Distribute metabolite") //.setMaster("local[4]")
        val sc = new SparkContext(conf)
        if (config.from_file) {
          run_calculation(sc, config, (0, 0), config.output_path.toString)
          sc.stop()
          sys.exit(0)
        }
        val range = if (config.calculate_all) {
          (0, calculate_number_of_combinations(config))
        } else {
          config.range
        }
        if (config.parts_size > 0) {
          val ranges = List.range(range._1, range._2, config.parts_size) ++ List(range._2)
          println(range)
          println(ranges)
          println(ranges zip ranges.tail)
          for ((part_range, i) <- (ranges zip ranges.tail).zipWithIndex) {
            val out_path = new File(config.output_path, "part" + i.toString)
            run_calculation(sc, config, part_range, out_path.toString)
            if (config.auto_copy){
              hdfs_get + out_path.toString + " " + temp_dir.toString !
              val out_dir = new File(temp_dir.toFile, out_path.toString)
              if (out_dir.exists){
                // TODO crating tar.gz archive and copy it in safe place
              }
            }

          }
        } else {
          run_calculation(sc, config, range, config.output_path.toString)
        }
        sc.stop()

      case None =>
        sys.exit(-1)
    }
  }
}
