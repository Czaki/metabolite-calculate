#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/exception_ptr.hpp>

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
  std::cout << "Usage: " << std::endl << name << " qsspn_file sfba_file [result_file] [options]" << std::endl;
  std::cout << "if result_file is not specified results is writen on stdout" << std::endl;
  std::cout << visible_options << "\n";
}

auto parse_options(int argc, const char ** argv){
  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
      ("qsspn_file", po::value<std::string>(), "path to qsspn file")
      ("sfba_file", po::value<std::string>(), "path to sfba file")
      ("result_file", po::value<std::string>(), "path to result file if absent result is printed on stdout");
  po::options_description general_options("General options");
  general_options.add_options()
      ("help,h", "produce help message");
  po::options_description mode_options("Mode options");
  mode_options.add_options()
      ("target", po::value<std::string>(), "optimization target")
      ("interactive", "use program in interactive mode")
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

  if (vm.count("help") || ( vm.count("qsspn_file") == 0 || vm.count("sfba_file") == 0)) {
    print_usage(visible_options, argv[0]);
    exit(1);
  }
  return vm;
}

int main(int argc, const char *argv[]) {
  auto vm = parse_options(argc, argv);
  if (vm.count("range")){
    std::pair<size_t, size_t> r = vm.at("range").as<std::pair<size_t, size_t>>();
    std::cout << r.first << " " << r.second << std::endl;
  }
  Metabolism met = Metabolism(vm.at("qsspn_file").as<std::string>(), vm.at("sfba_file").as<std::string>());
  if (vm.count("target")){
    if (!met.set_target(vm.at("target").as<std::string>()))
    {
      std::cerr << "invalid taget value" << std::endl;
      return 1;
    }
  }
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
  return 0;
}