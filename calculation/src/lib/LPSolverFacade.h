/* 
 * File:   LPSolverFacade.h
 * Author: jsroka
 *
 * Created on 6 września 2016, 01:32
 */

#ifndef LP_SOLVER_FACADE_H
#define LP_SOLVER_FACADE_H

#include "../glpk/src/glpk.h"
#include "LPSolverAbstract.h"

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

namespace PNFBA {

    /**
     * Facade to LP solver.
     */

    class LPSolverFacade : public LPSolverFacadeAbstract{
    public:
        LPSolverFacade(std::vector<std::string>& lp_system_row_names,
                std::vector<std::string>& lp_system_col_names,
                std::map<std::string, int>& lp_system_row_index,
                std::map<std::string, int>& lp_system_col_index,
                std::vector<int>& lp_system_i,
                std::vector<int>& lp_system_j,
                std::vector<double>& lp_system_val);
        virtual ~LPSolverFacade();
        LPSolverFacade(const LPSolverFacade& orig){partial_problem_for_copying = orig.partial_problem_for_copying;}


        std::pair<OptError, double> optimize(const Method &method_,
                                                     const std::string &objective,
                                                     std::vector<std::tuple<int, double, double> > &constraints_for_solver,
                                                     std::vector<bool> marking_for_debug);

    private:
        /* We don't want anyone to copy it. */ //NOPE
        //LPSolverFacade(const LPSolverFacade& orig);

        glp_prob* partial_problem_for_copying;
      std::map<std::string, glp_prob*> problem_map;
      std::vector<std::string> lp_system_row_names;
      std::vector<std::string> lp_system_col_names;
      std::map<std::string, int> lp_system_row_index;
      std::map<std::string, int> lp_system_col_index;
      std::vector<int> lp_system_i;
      std::vector<int> lp_system_j;
      std::vector<double> lp_system_val;
    };

}



#endif /* LP_SOLVER_FACADE_H */

