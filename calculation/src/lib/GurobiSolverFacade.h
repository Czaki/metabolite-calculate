//
// Created by czaki on 04.07.17.
//

#ifndef METABOLITE_GUROBISOLVERFASCADE_H
#define METABOLITE_GUROBISOLVERFASCADE_H
#ifdef GUROBI

#include <map>
#include <unordered_map>
#include "LPSolverAbstract.h"
#include "gurobi_c++.h"

namespace PNFBA {
class GurobiSolverFacade : public LPSolverFacadeAbstract
{
  GurobiSolverFacade(std::vector<std::string>& lp_system_row_names,
                     std::vector<std::string>& lp_system_col_names,
                     std::map<std::string, int>& lp_system_row_index,
                     std::map<std::string, int>& lp_system_col_index,
                     std::vector<int>& lp_system_i,
                     std::vector<int>& lp_system_j,
                     std::vector<double>& lp_system_val);

private:
  GRBEnv env;
  GRBModel model;

};
}

#endif // GUROBI
#endif //METABOLITE_GUROBISOLVERFASCADE_H
