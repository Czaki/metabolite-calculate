/*
 * File:   LPSolverFacade.cpp
 * Author: jsroka
 *
 * Created on 6 września 2016, 01:33
 */

#include "LPSolverFacade.h"
#include "../glpk/glpk.h"
#include <cstddef>
#include <cstdlib>
#include <memory>

int ill_conditioned;

namespace {
std::string error_code_name(int err_num) {
  switch (err_num) {
  case GLP_EBADB:
    return "GLP_EBADB   0x01  invalid basis";
  case GLP_ESING:
    return "GLP_ESING   0x02  singular matrix";
  case GLP_ECOND:
    return "GLP_ECOND   0x03  ill-conditioned matrix";
  case GLP_EBOUND:
    return "GLP_EBOUND  0x04  invalid bounds";
  case GLP_EFAIL:
    return "GLP_EFAIL   0x05  solver failed";
  case GLP_EOBJLL:
    return "GLP_EOBJLL  0x06  objective lower limit reached";
  case GLP_EOBJUL:
    return "GLP_EOBJUL  0x07  objective upper limit reached";
  case GLP_EITLIM:
    return "GLP_EITLIM  0x08  iteration limit exceeded";
  case GLP_ETMLIM:
    return "GLP_ETMLIM  0x09  time limit exceeded";
  case GLP_ENOPFS:
    return "GLP_ENOPFS  0x0A  no primal feasible solution";
  case GLP_ENODFS:
    return "GLP_ENODFS  0x0B  no dual feasible solution";
  case GLP_EROOT:
    return "GLP_EROOT   0x0C  root LP optimum not provided";
  case GLP_ESTOP:
    return "GLP_ESTOP   0x0D  search terminated by application";
  case GLP_EMIPGAP:
    return "GLP_EMIPGAP 0x0E  relative mip gap tolerance reached";
  case GLP_ENOFEAS:
    return "GLP_ENOFEAS 0x0F  no primal/dual feasible solution";
  case GLP_ENOCVG:
    return "GLP_ENOCVG  0x10  no convergence";
  case GLP_EINSTAB:
    return "GLP_EINSTAB 0x11  numerical instability";
  case GLP_EDATA:
    return "GLP_EDATA   0x12  invalid data";
  case GLP_ERANGE:
    return "GLP_ERANGE  0x13  result out of range";
  default:
    return "Unknown error";
  }
}
template <class T>
inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
  os << v.size();
  for (auto val : v) {
    os << " " << val;
  }
  return os;
}
}

namespace PNFBA {

LPSolverFacade::LPSolverFacade(std::vector<std::string> &lp_system_row_names_,
                               std::vector<std::string> &lp_system_col_names_,
                               std::map<std::string, int> &lp_system_row_index_,
                               std::map<std::string, int> &lp_system_col_index_,
                               std::vector<int> &lp_system_i_,
                               std::vector<int> &lp_system_j_,
                               std::vector<double> &lp_system_val_) {

  // GLPK can be used to solve the following problems:
  // maximize z = 10x1 + 6x2 + 4x3
  // subject to:
  //  p =   x1 +  x2 +  x3
  //  q = 10x1 + 4x2 + 5x3
  //  r =  2x1 + 2x2 + 6x3

  //      x1   x2  x3
  //  p =  1 + 1 + 1
  //  q = 10 + 4 + 5
  //  r =  2 + 2 + 6

  // and bounds of variables:
  //  −∞ < p ≤ 100
  //  −∞ < q ≤ 600
  //  −∞ < r ≤ 300
  //  0 ≤ x1 < +∞
  //  0 ≤ x2 < +∞
  //  0 ≤ x3 < +∞
  // where p, q, r are auxiliary variables (rows), and x 1 , x 2 , x 3 are
  // structural variables (columns).
  this->lp_system_row_names = lp_system_row_names_;
  this->lp_system_col_names = lp_system_col_names_;
  this->lp_system_row_index = lp_system_row_index_;
  this->lp_system_col_index = lp_system_col_index_;
  this->lp_system_i = lp_system_i_;
  this->lp_system_j = lp_system_j_;
  this->lp_system_val = lp_system_val_;

  partial_problem_for_copying = glp_create_prob();

  // najpierw nadajemy nazwy symboliczne dla reakcji (kolumny wedlug glpk)
  glp_add_cols(partial_problem_for_copying, (int)lp_system_col_names.size());
  for (size_t i = 0; i < lp_system_col_names.size(); ++i) {
    glp_set_col_name(partial_problem_for_copying, (int)i + 1,
                     const_cast<char *>(lp_system_col_names.at(i).c_str()));
  }

  // potem nazwy symboliczne dla substancji (wiersze wedlug glpk)
  glp_add_rows(partial_problem_for_copying, (int)lp_system_row_names.size());
  for (size_t i = 0; i < lp_system_row_names.size(); ++i) {
    glp_set_row_name(partial_problem_for_copying, (int)i + 1,
                     const_cast<char *>(lp_system_row_names.at(i).c_str()));
    glp_set_row_bnds(partial_problem_for_copying, (int)i + 1, GLP_FX, 0,
                     0); // ustawiamy wiezy dla kazdej substancji
  }

  if (0 != glp_check_dup(
               (int)lp_system_row_names.size(), (int)lp_system_col_names.size(),
               (int)lp_system_val.size() - 1, &lp_system_i[0],
               &lp_system_j[0])) { // 0 a nie od 1, mimo, ze na 0 jest gruadian,
                                   // ale wyglada na to ze takie jest API
    std::cerr << "LP problem corruption. Exiting.";
    std::exit(1);
  }

  glp_load_matrix(partial_problem_for_copying, lp_system_i.size() - 1,
                  &lp_system_i[0], &lp_system_j[0], &lp_system_val[0]);
  glp_sort_matrix(partial_problem_for_copying); // trudno z dokumentacji
                                                // wywnioskowac po co to jest, w
                                                // przykladzie w dokumentacji
                                                // tego nie

  // creates index allowing to find rows and/or columns by their names
  // If the name index does not exist, the application should not call the
  // routines glp_find_row and glp_find_col
  // this was before done for each optimization, JS moved it to initialization
  glp_create_index(partial_problem_for_copying);
  //!!!TODO??? trzeba sprawdzic czy skopiowanie tego tutaj nie spowolnilo i
  //!nadal dziala
}

LPSolverFacade::~LPSolverFacade() {
  glp_delete_prob(partial_problem_for_copying);
}

std::pair<OptError, double> LPSolverFacade::optimize(const Method &method_,
    const std::string &objective,
    std::vector<std::tuple<int, double, double>> &constraints_for_solver,
    std::vector<bool> marking_for_debug) {

  // kopiujemy definicje problemu ktora utworzylismy w konstruktorze (jeszcze
  // jest bez czesci wiezow)
  glp_prob *root = glp_create_prob();
  glp_copy_prob(
      root, partial_problem_for_copying,
      GLP_ON); // If it is GLP_ON, the routine also copies all symbolic names
  glp_set_obj_dir(root, GLP_MAX); // maksymalizujemy
  glp_create_index(root);

  // ustawiamy wiezy i funkcje celu
  if (objective == "") {
    std::cerr << "ERROR: Empty objective\n";
    std::exit(-1);
  }

  // celem optymalizacji jest przeplyw przez reakcje
  if (lp_system_col_index.count(objective) > 0) {
    glp_set_obj_coef(root, lp_system_col_index.at(objective),
                     1.0); ////zmienia definicje funkcji celu
                           // celem optymalizacji jest substancja
  } else if (lp_system_row_index.count(objective) > 0) {
    int length =
        glp_get_mat_row(root, glp_find_row(root, objective.c_str()), NULL,
                        NULL); // glp_find_row zwraca numer dla row, a
                               // glp_get_mat_row zwraca dlugosc
    // drugie wywolanie wpisuje do tablic indeksy niezerowych elementow i ich
    // wartosci
    std::unique_ptr<int[]> indices(new int[length + 1]);
    std::unique_ptr<double[]> values(new double[length + 1]);
    glp_get_mat_row(root, lp_system_row_index.at(objective), indices.get(),
                    values.get());

    for (int i = 1; i < length + 1; ++i) {
      glp_set_obj_coef(root, indices[i],
                       values[i]); // zmienia definicje funkcji celu
    }
    glp_set_row_bnds(root, lp_system_row_index.at(objective), GLP_FR, 0,
                     0); //!!!TODO??? czemu jest 0, 0 tak jak poprzednio, jaki
                         //!sens ma 0, 0 z FR?
                         // GLP_FR −∞ < x < +∞ Free (unbounded) variable
    // GLP_LO lb ≤ x < +∞ Variable with lower bound
    // GLP_UP −∞ < x ≤ ub Variable with upper bound
    // GLP_DB lb ≤ x ≤ ub Double-bounded variable
    // GLP_FX lb = x Fixed variable
  } else {
    std::cerr << "ERROR: Unknown objective: '" << objective << "'\n";
    std::exit(-1);
  }

  // TODO tu byla para a jest tuple
  for (auto bnd : constraints_for_solver) {
    if (std::get<1>(bnd) == std::get<2>(bnd)) {
      glp_set_col_bnds(root, std::get<0>(bnd), GLP_FX, std::get<1>(bnd),
                       std::get<2>(bnd));
    } else {
      glp_set_col_bnds(root, std::get<0>(bnd), GLP_DB, std::get<1>(bnd),
                       std::get<2>(bnd));
    }
  }
  {
    glp_bfcp parm;
    glp_get_bfcp(root, &parm);
    parm.type = GLP_BF_GR;
    glp_set_bfcp(root, &parm);
  }
  double result = 0.;
  bool error = false;
  glp_prob *root2 = glp_create_prob();
  glp_copy_prob(root2, root, GLP_ON);
  int err_num;
  switch (method_) {
  case SIMPLEX:
    ill_conditioned = false;
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.meth = GLP_DUAL;
    parm.it_lim = 500000;
    parm.presolve = GLP_ON;
      parm.msg_lev=GLP_MSG_OFF;
    if (glp_simplex(root, &parm) == 0 && glp_get_status(root) == GLP_OPT) {
      // successfully completed
      result = glp_get_obj_val(root);
      break; // Jak nie ma break to wykonuje dalej, może wejsc w następne
             // gałęzie.
    } else {
      result = 0.0;
      std::cerr << "SIMPLEX error; objective: " << objective << " marking "
                << marking_for_debug;
      std::cerr << " error type: " << error_code_name(glp_get_status(root))
                << std::endl;
      break;
      std::swap(root, root2);
    }
  case INTERIOR:
    ill_conditioned = false;
    if ((err_num = glp_interior(root, nullptr)) == 0 &&
        glp_ipt_status(root) == GLP_OPT) {
      // successfully completed
      result = glp_get_obj_val(root);
    } else {
      std::cerr << "INTERIOR error; objective: " << objective << " marking "
                << marking_for_debug;
      std::cerr << " error type: " << error_code_name(err_num) << std::endl;
      error = true;
      result = 0.0;
    }
    break;
  case EXACT: {
    if (glp_exact(root, nullptr) == 0 && glp_get_status(root) == GLP_OPT) {
      // successfully completed
      result = glp_get_obj_val(root);
    } else {
      error = true;
      result = 0.0;
    }
  } break;
  default:
    std::cout << "We shouldn't be here...";
    std::exit(1);
  }

  OptError err((bool)ill_conditioned, error);

  glp_delete_prob(root);
  glp_delete_prob(root2);

  return std::make_pair(err, result);
}
}

std::ostream & operator<<(std::ostream &os, const PNFBA::OptError &oe) {
  os << "ill: " << oe.ill_condition_ << " err: " << oe.other_error_;
  return os;
}

std::istream & operator>>(std::istream &is, PNFBA::OptError &oe) {
  std::string ill, err;
  bool ill_val, err_val;
  is >> ill >> ill_val >> err >> err_val;
  oe = PNFBA::OptError(ill_val, err_val);
#ifdef DEBUG
  if (ill != "ill:") {
    throw std::logic_error("Wrong text format meet \"" + ill +
                           "\" expected \"ill\"");
  }
  if (err != "err:") {
    throw std::logic_error("Wrong text format meet \"" + err +
                           "\" expected \"err\"");
  }
#endif
  return is;
}
