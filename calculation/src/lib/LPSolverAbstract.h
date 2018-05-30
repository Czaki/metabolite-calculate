//
// Created by czaki on 04.07.17.
//

#ifndef METABOLITE_LPSOLVERABSTRACT_H
#define METABOLITE_LPSOLVERABSTRACT_H

#include <utility>
#include <iostream>
#include <vector>

namespace PNFBA {
class OptError;
}

std::ostream& operator<<(std::ostream& os, const PNFBA::OptError & oe);
std::istream& operator>>(std::istream& is, PNFBA::OptError & oe);



namespace PNFBA {

/**
 * Facade to LP solver.
 */

enum Method
{
    SIMPLEX,
    INTERIOR,
    EXACT,
    UNKNOWN
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

class LPSolverFacadeAbstract
{
public:
  virtual std::pair<OptError, double> optimize(const Method &method_,
                                               const std::string &objective,
                                               std::vector <std::tuple<int, double, double>> &constraints_for_solver,
                                               std::vector<bool> marking_for_debug) = 0;
};
}


#endif //METABOLITE_LPSOLVERABSTRACT_H
