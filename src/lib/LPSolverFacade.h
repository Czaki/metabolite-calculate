/* 
 * File:   LPSolverFacade.h
 * Author: jsroka
 *
 * Created on 6 września 2016, 01:32
 */

#ifndef LP_SOLVER_FACADE_H
#define LP_SOLVER_FACADE_H

#include "../glpk/glpk.h"
//#include "Metabolism.h"

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
namespace PNFBA {
    class OptError;

}
std::ostream& operator<<(std::ostream& os, const PNFBA::OptError & oe);
std::istream& operator>>(std::istream& is, PNFBA::OptError & oe);

namespace PNFBA {

    /**
     * Facade to LP solver.
     */

    enum Method {
      SIMPLEX,
      INTERIOR,
      EXACT,
      UNKNOWN
    };

    class LPSolverFacade {
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
      std::vector<std::string> lp_system_row_names;
      std::vector<std::string> lp_system_col_names;
      std::map<std::string, int> lp_system_row_index;
      std::map<std::string, int> lp_system_col_index;
      std::vector<int> lp_system_i;
      std::vector<int> lp_system_j;
      std::vector<double> lp_system_val;
    };

    class OptError {
    private:
        bool ill_condition_;
        bool other_error_;
    public:

        OptError(bool ill, bool err) : ill_condition_(ill), other_error_(err) {
        };

        OptError(std::pair<bool, bool> p) : ill_condition_(p.first), other_error_(p.second) {
        };

        OptError() : ill_condition_(false), other_error_(false) {
        };

        bool ill_condition() {
            return ill_condition_;
        };

        bool normal_error() {
            return other_error_;
        };
        friend std::ostream& (::operator<<)(std::ostream& os, const OptError & oe);
        friend std::istream& (::operator>>)(std::istream& is, OptError & oe);

        OptError& operator+=(const OptError & oe) {
            this->ill_condition_ |= oe.ill_condition_;
            this->other_error_ |= oe.other_error_;
            return *this;
        }
        friend OptError operator+(const OptError & oe1, const OptError & oe2);
    };

    inline OptError operator+(const OptError & oe1, const OptError & oe2) {
        return OptError(oe1.ill_condition_ | oe2.ill_condition_, oe1.other_error_ | oe2.other_error_);
    }
    
    
}



#endif /* LP_SOLVER_FACADE_H */

