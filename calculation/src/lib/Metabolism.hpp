//
// Created by Grzegorz Bokota on 04.03.2017.
//

#ifndef METABOLITE_METABOLISM_HPP
#define METABOLITE_METABOLISM_HPP
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <set>
#include "LPSolverFacade.h"

typedef uint32_t counter_type;

class FileFormatException : public std::logic_error {
public:
  FileFormatException(std::string s) : std::logic_error(s){};
};

class Enzyme {
public:
  Enzyme(std::string name, std::vector<std::pair<counter_type , std::pair<double, double>>> boundaries,
         std::vector<std::pair<std::string, double>> elements) :
          name_(name), boundaries_(boundaries), elements_(elements) {};
  std::vector<counter_type > get_threshold_values() const {
    std::vector<counter_type > res;
    for (auto & el : boundaries_){
      res.push_back(el.first);
    }
    return res;
  }

  std::vector<std::string> get_reaction_list() const {
    std::vector<std::string> res;
    for (auto & el : elements_){
      res.push_back(el.first);
    }
    return res;
  }
  auto operator()(counter_type c) const {
    std::pair<double, double> res(-1,1);
    for (auto &el : boundaries_) {
      if (el.first <= c) {
        res = el.second;
      } else {
        return res;
      }
    }
    return res;
  }

private:
  std::string name_;
  std::vector<std::pair<counter_type, std::pair<double, double>>> boundaries_;
  std::vector<std::pair<std::string, double>> elements_;
};

class SFBA {
public:
  typedef std::vector<std::tuple<int, double, double>> ProblemConstraint;
  SFBA(std::string sfba_path, std::string ext_tag, const std::vector<Enzyme> &enzyme_vec);
  virtual  ~SFBA() {
    free(this->lp_solver_);
  }
  void parse_lp_system(std::istream &metabolism_sfba,
                       const std::string &ext_tag);
  void parse_linear_expression(const std::string &expression, const int j,
                               const std::string &ext_tag, double sign,
                               int *index);
  double optimize(const std::string & objective, ProblemConstraint &constraints_for_solver){
    return lp_solver_->optimize(PNFBA::SIMPLEX, objective, constraints_for_solver, std::vector<bool>()).second;
  }
  ProblemConstraint prepareProblemConstraints(const std::vector<counter_type >);


  void print_info(){
    std::cerr << "lp_system_i_: " << lp_system_i_.size() << std::endl;
    std::cerr << "lp_system_j_: " << lp_system_j_.size() << std::endl;
    std::cerr << "lp_system_row_names_: " << lp_system_row_names_.size() << std::endl;
    std::cerr << "lp_system_col_names_: " << lp_system_col_names_.size() << std::endl;
    std::cerr << "lp_system_row_index_: " << lp_system_row_index_.size() << std::endl;
    std::cerr << "lp_system_col_index_: " << lp_system_col_index_.size() << std::endl;
  }

private:
  std::vector<int> lp_system_i_;
  std::vector<int> lp_system_j_;
  std::vector<double> lp_system_val_;
  std::vector<std::string> lp_system_row_names_;
  std::vector<std::string> lp_system_col_names_;
  std::map<std::string, int> lp_system_row_index_;
  std::map<std::string, int> lp_system_col_index_;
  std::map<std::string, std::pair<double, double>> default_constraints_;
  std::map<std::string, std::pair<size_t, const Enzyme *>> reaction_enzyme_map_;
  PNFBA::LPSolverFacade * lp_solver_;
};

class Metabolite {
public:
  Metabolite(std::string name, std::string goal, std::vector<std::pair<double, counter_type>> mapping):
          name_(name), goal_(goal), mapping_(mapping){};
  auto operator()(double c) const {
    counter_type res = 0;
    for (auto &el : mapping_) {
      if (el.first <= c) {
        res = el.second;
      } else {
        return res;
      }
    }
    return res;
  };
  size_t number_of_levels() const { return mapping_.size(); }

  auto goal() { return this->goal_;}

private:
  std::string name_;
  std::string goal_;
  std::vector<std::pair<double, counter_type>> mapping_;
};



class Metabolism {
public:
  typedef std::vector<uint8_t> Marking;
  typedef SFBA::ProblemConstraint ProblemConstraint;
  Metabolism(std::string qsspn_file_path, std::string sfba_file_path);
  virtual ~Metabolism(){
    free(this->solver);
  };
  void set_range(size_t begin, size_t end){
    this->range_ = std::make_pair(begin,end);
  }
  void calculateRange(std::ostream &result_file, size_t begin, size_t end);
  void print_targets(std::ostream &os){
    for(auto &el : targets_set_){
      os << el << std::endl;
    }
  }

private:
  std::string model_name_; /*! model name  */
  std::string ext_tag_;
  std::vector<Metabolite> metabolites_;
  std::vector<Enzyme> enzymes_;
  std::set<std::string> targets_set_;
  std::pair<size_t, size_t> range_;
  SFBA * solver;
};

#endif // METABOLITE_METABOLISM_HPP
