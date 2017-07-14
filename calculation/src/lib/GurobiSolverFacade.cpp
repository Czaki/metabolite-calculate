//
// Created by czaki on 04.07.17.
//

#ifdef GUROBI
#include "GurobiSolverFacade.h"
#include <iostream>
#include <limits>
#include <tuple>

const std::string metabolite_suffix = "+consume";
namespace PNFBA {
GurobiSolverFacade::GurobiSolverFacade(
    std::vector<std::string> &lp_system_row_names_,
    std::vector<std::string> &lp_system_col_names_,
    std::map<std::string, int> &lp_system_row_index_,
    std::map<std::string, int> &lp_system_col_index_,
    std::vector<int> &lp_system_i_, std::vector<int> &lp_system_j_,
    std::vector<double> &lp_system_val_)
    : model(env), lp_system_col_index(lp_system_col_index_),
      lp_system_row_index(lp_system_row_index_),
      lp_system_row_names(lp_system_row_names_),
      lp_system_col_names(lp_system_col_names_), lp_system_i(lp_system_i_),
      lp_system_j(lp_system_j_), lp_system_val(lp_system_val_),
      lp_system_metabolite_index_(lp_system_col_index),
      lp_system_reaction_index_(lp_system_row_index) {
  // col - reakcje
  // row - metabolity
  std::unordered_map<std::string, GRBVar> variable_map;
  std::unordered_map<std::string, GRBLinExpr> equation_map;
  for (auto &name : lp_system_col_names) {
    variable_map[name] =
        model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, name);
  }
  for (auto &name : lp_system_row_names) {
    equation_map[name] = 0.0;
  }
  for (size_t i = 1; i < lp_system_i.size(); i++) {
    std::string name = lp_system_row_names[lp_system_i[i] - 1];
    equation_map[name] += lp_system_val[i] *
                          variable_map[lp_system_col_names[lp_system_j[i] - 1]];
  }
  for (auto &name : lp_system_row_names) {
    std::string consume_name = name + metabolite_suffix;
    equation_map[name] += -1 *
                         model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0,
                                      GRB_CONTINUOUS, consume_name);
  }
  for (auto &equation : equation_map) {
    model.addConstr(equation.second == 0, equation.first);
  }
  model.update();
}

std::pair<OptError, double> GurobiSolverFacade::optimize(
    const Method &method_, const std::string &objective,
    std::vector<std::tuple<int, double, double>> &constraints_for_solver,
    std::vector<bool> marking_for_debug) {
  (void)method_;

  if (objective == "") {
    std::cerr << "ERROR: Empty objective\n";
    std::exit(-1);
  }
  GRBModel model(this->model);
  GRBVar * vars = this->model.getVars();
  int vars_num = this->model.get(GRB_IntAttr_NumVars);
  std::vector<std::string> var_names;
  for (int i=0; i<vars_num; ++i){
    var_names.push_back(vars[i].get(GRB_StringAttr_VarName));
  }
  if (lp_system_col_index.count(objective) > 0) {
    GRBVar var = model.getVarByName(objective);
    GRBLinExpr expr = var;
    model.setObjective(expr, GRB_MAXIMIZE);
  } else if (lp_system_row_index.count(objective) > 0) {
    GRBVar var = model.getVarByName(objective + metabolite_suffix);
    GRBLinExpr expr = var;
    model.setObjective(expr, GRB_MAXIMIZE);
  } else {
    std::cerr << "ERROR: Objective not found\n";
    std::exit(-1);
  }
  for (size_t i = 0; i < constraints_for_solver.size(); ++i) {
    int index;
    double lb, ub;
    std::tie(index, lb, ub) = constraints_for_solver[i];
    auto name = lp_system_col_names[index-1];
    if (name == objective){
      continue;
    }
    GRBVar var = model.getVarByName(name);
    var.set(GRB_DoubleAttr_LB, lb);
    var.set(GRB_DoubleAttr_UB, ub);
  }
  for(auto & name: lp_system_row_names){
    if (name == objective)
      continue;
    GRBVar var = model.getVarByName(name+metabolite_suffix);
    var.set(GRB_DoubleAttr_LB, 0);
    var.set(GRB_DoubleAttr_UB, 0);
  }
  model.set(GRB_IntParam_OutputFlag , 0);
  model.write("test1_"+objective+".mps");
  model.optimize();


  return std::make_pair(OptError(false, false), model.get(GRB_DoubleAttr_ObjVal));
}
}
#endif
