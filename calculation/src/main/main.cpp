#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/lexical_cast.hpp>

#include "../lib/Metabolism.hpp"

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 6){
        std::cerr << "Usage " << argv[0] << " qsspn_file sfba_file result_file [-str|(begin end)]" << std::endl;
        exit(-1);
    }
  std::cerr << "argc: " << argc << std::endl;
  auto met = Metabolism(argv[1], argv[2]);
  if (argc == 4 && strcmp(argv[3], "-str")==0) {
    while (!std::cin.eof()) {
      std::string line;
      if (!std::getline(std::cin, line)){
        return 0;
      }
      size_t pos = line.find(',');
      size_t begin, end;
      begin = boost::lexical_cast<size_t>(line.substr(1, pos - 1));
      end = boost::lexical_cast<size_t>(line.substr(pos + 1, line.length() - pos - 2));
      //std:: cin >> begin >> end;
      met.set_range(begin, end);
      std::cerr << "Start " << line << " " << begin << " " << end << "\n";
      /*for (size_t i = 0; i< end-begin; i++){
        std::cout << "1 1 1 1 8 1 1 1 1 1 1 1 1 9\n";
      }*/
      met.getProblemConstraintList(std::cout);
    }
    return 0;
  }
  if (argc == 6){
    size_t begin, end;
    std::stringstream ss(argv[4]);
    ss >> begin;
    ss.str(argv[5]);
    ss.seekg(0);
    ss >> end;
    met.set_range(begin, end);
  }

  met.print_targets(std::cout);
  std::ofstream result_file(argv[3]);
  met.getProblemConstraintList(result_file);
  result_file.close();

  return 0;
}