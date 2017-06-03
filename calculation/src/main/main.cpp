#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#ifdef MPI_BUILD
#include <mpi.h>
const size_t chunk_szie = 200;
#endif

#include "../lib/Metabolism.hpp"

namespace po = boost::program_options;

namespace std {
istream & operator>>(istream &is, pair<size_t, size_t> &p){
  std::string line;
  getline(is, line);
  size_t pos = line.find(',');
  p.first = boost::lexical_cast<size_t>(line.substr(0, pos));
  p.second = boost::lexical_cast<size_t>(line.substr(pos + 1, line.length() - pos));
  return is;
}
}

void print_usage(po::options_description visible_options, std::string name){
#ifdef MPI_BUILD
  std::cout << "Usage: " << std::endl << name << " qsspn_file sfba_file result_folder [options]" << std::endl;
#else
  std::cout << "Usage: " << std::endl << name << " qsspn_file sfba_file [result_file] [options]" << std::endl;
#endif
  std::cout << "if result_file is not specified results is writen on stdout" << std::endl;
  std::cout << visible_options << "\n";
}

auto parse_options(int argc, const char ** argv){
  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
      ("qsspn_file", po::value<std::string>(), "path to qsspn file")
      ("sfba_file", po::value<std::string>(), "path to sfba file")
#ifdef MPI_BUILD
      ("result_folder", po::value<std::string>(), "path to result folder");
#else
      ("result_file", po::value<std::string>(), "path to result file if absent result is printed on stdout");
#endif
  po::options_description general_options("General options");
  general_options.add_options()
      ("help,h", "produce help message");
  po::options_description mode_options("Mode options");
  mode_options.add_options()
      ("target", po::value<std::string>(), "optimization target")
#ifndef MPI_BUILD
      ("interactive", "use program in interactive mode")
#endif
      ("range", po::value<std::pair<size_t, size_t>>(), "range to calculate");
  po::options_description cmd_options;
  cmd_options.add(hidden_options).add(general_options).add(mode_options);
  po::options_description visible_options("Allowed options");
  visible_options.add(general_options).add(mode_options);
  po::positional_options_description p;
  p.add("qsspn_file", 1);
  p.add("sfba_file", 1);
  p.add("result_file", 1);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(cmd_options).positional(p).run(), vm);
    po::notify(vm);
  } catch (std::exception const & e){
    std::cout << "Argument error: ";
    std::cout << e.what() << std::endl;
    print_usage(visible_options, argv[0]);
    exit(2);
  }

  if (vm.count("help")) {
    print_usage(visible_options, argv[0]);
    exit(1);
  }
#ifdef MPI_BUILD
  std::vector<std::string> need  = {"qsspn_file", "sfba_file", "result_folder"};
#else
  std::vector<std::string> need  = {"qsspn_file", "sfba_file"};
#endif
  for (auto name : need){
    if (vm.count(name) == 0){
      std::cout << name << " not set" << std::endl;
      print_usage(visible_options, argv[0]);
    }
  }
  return vm;
}

#ifdef MPI_BUILD

#define DATA_RANGE 10
#define CALC_END  11
#define MASTER_RANK 0

void control_server(Metabolism & met){
  const std::pair<size_t, size_t> range = met.get_range();
  int mpi_size;
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  size_t begin = range.first;
  begin += chunk_szie * (mpi_size - 1);
  while (begin < range.second){
    char buff[100];
    MPI_Status status;
    MPI_Recv(buff, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    size_t send_range[2] = {begin, begin + chunk_szie};
    MPI_Send(send_range, sizeof(size_t)*2, MPI_CHAR, status.MPI_SOURCE, DATA_RANGE, MPI_COMM_WORLD);
  }
  while (mpi_size > 1){
    char buff[100];
    MPI_Status status;
    MPI_Recv(buff, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Send(buff, 1, MPI_CHAR, status.MPI_SOURCE, CALC_END, MPI_COMM_WORLD);
    mpi_size--;
  }
  MPI_Finalize();
}
#endif

int main(int argc, char *argv[]) {
#ifdef MPI_BUILD
  MPI_Init(&argc, &argv);
  int mpi_size, mpi_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  int num_len = snprintf(nullptr, 0, "%d", mpi_size) + 1;
#endif
  auto vm = parse_options(argc, (const char **) argv);
  Metabolism met = Metabolism(vm.at("qsspn_file").as<std::string>(), vm.at("sfba_file").as<std::string>());
  if (vm.count("target")){
    if (!met.set_target(vm.at("target").as<std::string>()))
    {
      std::cerr << "invalid taget value" << std::endl;
      return 1;
    }
  }
#ifdef MPI_BUILD
  if (mpi_rank != MASTER_RANK) {
    char number[num_len + 5];
    char format_string[30];
    sprintf(format_string, "%%0%dd", num_len);
    sprintf(number, format_string, mpi_rank);
    std::ofstream os(vm.at("result_folder").as<std::string>() + "/part" + number + ".txt");
    if (!os.is_open()) {
      // TODO send message about error;
      MPI_Finalize();
      return 1;
    }
    size_t end = chunk_szie * mpi_rank;
    size_t begin = end - chunk_szie;
    met.calculateRange(os, begin, end);
    while (true){
      size_t range[2];
      MPI_Status status;
      MPI_Send(range, 1, MPI_CHAR, MASTER_RANK, DATA_RANGE, MPI_COMM_WORLD);
      MPI_Recv(range, sizeof(size_t)*2, MPI_CHAR, MASTER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (status.MPI_TAG == CALC_END)
        break;
      else
        met.calculateRange(os, range[0], range[1]);
    }
    MPI_Finalize();
  } else {
    // Process with rank zero is control server
    control_server(met);
  }
#else
  std::ofstream ofs;
  std::ostream * os;
  if (vm.count("result_file")){
    ofs.open(vm.at("result_file").as<std::string>());
    os = &ofs;
  } else {
    os = & std::cout;
  }
  if (vm.count("interactive")) {
    while (!std::cin.eof()) {
      std::string line;
      if (!std::getline(std::cin, line)){
        return 0;
      }
      std::pair<size_t, size_t> range;
      size_t pos = line.find(',');
      size_t begin, end;
      try {
        begin = boost::lexical_cast<size_t>(line.substr(1, pos - 1));
        end = boost::lexical_cast<size_t>(line.substr(pos + 1, line.length() - pos - 2));
      } catch (std::exception){
        std::cerr << "wrong range: " << line << std::endl;
        continue;
      }
      if (begin >= end ){
        std::cerr << "wrong range: " << line << std::endl;
        continue;
      }
      met.calculateRange(*os, begin, end);
    }
  } else {
    if (vm.count("range")) {
      std::pair<size_t, size_t> r = vm.at("range").as<std::pair<size_t, size_t>>();
      met.set_range(r);
    }
    met.calculateRange(*os, 0, 0);
  }
  if (vm.count("result_file")){
    ofs.close();
  }

#endif
  return 0;
}