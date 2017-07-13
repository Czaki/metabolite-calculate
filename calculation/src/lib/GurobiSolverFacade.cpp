//
// Created by czaki on 04.07.17.
//

#ifdef GUROBI
#include "GurobiSolverFacade.h"
#include <limits>
#include <iostream>

namespace PNFBA {
GurobiSolverFacade::GurobiSolverFacade(
    std::vector<std::string> &lp_system_row_names,
    std::vector<std::string> &lp_system_col_names,
    std::map<std::string, int> &lp_system_row_index,
    std::map<std::string, int> &lp_system_col_index,
    std::vector<int> &lp_system_i, std::vector<int> &lp_system_j,
    std::vector<double> &lp_system_val)
    : model(env) {
  // col - reakcje
  // row - metabolity
  std::unordered_map<std::string, GRBVar> variable_map;
  std::unordered_map<std::string, GRBLinExpr> equation_map;
  for(auto & name : lp_system_col_names){
    variable_map[name] = model.addVar(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), 0.0, GRB_CONTINUOUS, name);
  }
  for(auto & name : lp_system_row_names){
    equation_map[name] = 0.0;
  }
  for (size_t i=1; i < lp_system_i.size(); i++){
    std::string name = lp_system_row_names[lp_system_i[i]-1];
    if (name == "HC00178_c"){
      std::cout << lp_system_val[i] << " " << lp_system_col_names[lp_system_j[i]-1] << std::endl;
    }
    equation_map[name] += lp_system_val[i] * variable_map[lp_system_col_names[lp_system_j[i]-1]];
  }
  for (auto & equation : equation_map){
    model.addConstr(equation.second == 0, equation.first);
  }

}
}
#endif
