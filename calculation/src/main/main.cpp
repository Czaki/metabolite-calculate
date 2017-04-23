#include <iostream>
#include <sstream>

#include "../lib/Metabolism.hpp"

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 6){
        std::cerr << "Usage " << argv[0] << " qsspn_file sfba_file result_file [begin end]" << std::endl;
        exit(-1);
    }
    auto met = Metabolism(argv[1], argv[2]);
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
  met.getProblemConstraintList(argv[3]);
    return 0;
}