//
// Created by Grzegorz Bokota on 04.03.2017.
//
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include "Metabolism.hpp"

namespace {

template <class T>
inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
  //os << v.size();
  for (auto val : v) {
    os << " " << val;
  }
  return os;
}

template <class T, class K>
inline std::ostream &operator<<(std::ostream &os,
                                const std::tuple<T, K, K> &v) {
  os << "(" << std::get<0>(v) << " " << std::get<1>(v) << " " << std::get<2>(v)
     << ")";
  return os;
}
std::vector<std::tuple<std::string, counter_type, counter_type, uint8_t>>
readSubstances(std::istream &s) {
  std::string line = "";
  std::vector<std::tuple<std::string, counter_type, counter_type, uint8_t>> res;
  do {
    std::getline(s, line);
    boost::trim(line);
  } while (line == "SUBSTANCES" || s.eof());
  if (s.eof()) {
    throw FileFormatException("end of file instead of SUBSTANCES\n");
  }

  std::stringstream ss;
  while (true) {
    std::getline(s, line);
    boost::trim(line);
    if (line == "END") {
      break;
    }
    if (s.eof()) {
      throw FileFormatException("SUBSTANCES finished without \"END\"\n");
    }
    ss.str(line);
    std::string name;
    counter_type state, max;
    uint8_t type;
    ss >> name >> state >> max >> type;
    res.push_back(std::make_tuple(name, state, max, type));
  }
  res.shrink_to_fit();
  return res;
};

struct metabolite_struct {
  std::string name;
  std::string goal;
  std::vector<std::pair<double, counter_type>> thresholds;
};

struct enzyme_struct {
  std::string name;
  std::vector<std::pair<counter_type, std::pair<double, double>>> boundaries;
  std::vector<std::pair<std::string, double>> elements;
};

struct metabolite_data {
  /*!
   * struct metabolite_data
   */
  std::string model_name; /*! model name  */
  std::string ext_tag;
  std::vector<Metabolite> metabolites;
  std::vector<Enzyme> enzymes;
  std::set<std::string> targets_set;
};

Metabolite readMetabolite(std::istream &s) {
  metabolite_struct res;
  std::string activity;
  int thr_count;
  s >> res.name >> res.goal;
  s >> activity >> thr_count;
  if (activity != "ACTIVITY") {
    throw FileFormatException("metabolite read error " +
                              std::to_string(__LINE__));
  }
  for (int i = 0; i < thr_count; i++) {
    counter_type val;
    double thr;
    s >> thr >> val;
    res.thresholds.push_back(std::make_pair(thr, val));
  }
  std::string end1, end2;
  s >> end1 >> end2;
  if (end1 != "END" || end2 != "END") {
    throw FileFormatException("metabolite read error " +
                              std::to_string(__LINE__));
  }

  return Metabolite(res.name, res.goal, res.thresholds);
}

auto readEnzyme(std::istream &s) {
  enzyme_struct res;
  std::string type, tag, name;
  int count;
  counter_type thr;
  double lower, upper, multip;
  s >> res.name >> type;
  s >> tag >> count;
  if (tag != "ACTIVITY") {
    throw FileFormatException("metabolite read error " +
                              std::to_string(__LINE__));
  }
  for (int i = 0; i < count; i++) {
    s >> thr >> lower >> upper;
    res.boundaries.push_back(std::make_pair(thr, std::make_pair(lower, upper)));
  }
  s >> tag;
  if (tag != "END") {
    throw FileFormatException("metabolite read error " +
                              std::to_string(__LINE__));
  }
  if (type == "list") {
    s >> tag >> count;
    for (int i = 0; i < count; i++) {
      s >> name >> multip;
      res.elements.push_back(std::make_pair(name, multip));
    }
    s >> tag;
    if (tag != "END") {
      throw FileFormatException("metabolite read error " +
                                std::to_string(__LINE__));
    }
  } else {
    res.elements.push_back(std::make_pair(type, 1.0));
  }
  s >> tag;
  if (tag != "END") {
    throw FileFormatException("metabolite read error " +
                              std::to_string(__LINE__));
  }
  return Enzyme(res.name, res.boundaries, res.elements);
}

metabolite_data readMetaboliteData(std::istream &s) {
  std::string token;
  metabolite_data res;
  s >> token;
  if (token != "MODEL") {
    throw FileFormatException("metabolite read error " +
                              std::to_string(__LINE__));
  }
  s >> res.model_name;
  s >> token;
  if (token != "EXT_TAG") {
    throw FileFormatException("metabolite read error " +
                              std::to_string(__LINE__));
  }
  s >> res.ext_tag;
  while (s.good()) {
    s >> token;
    if (token == "METABOLITE") {
      res.metabolites.push_back(readMetabolite(s));
    } else if (token == "ENZYME") {
      res.enzymes.push_back(readEnzyme(s));
    } else if (token == "END") {
      break;
    } else {
      throw FileFormatException("metabolite read error " +
                                std::to_string(__LINE__));
    }
  }
  return res;
};

void skipReactions(std::istream &s) {
  int blocks_count = 1;
  std::string line;
  while (blocks_count > 1) {
    std::getline(s, line);
    boost::trim(line);
    if (line == "END") {
      blocks_count--;
      continue;
    }
    if (line.substr(0, 11) == "INTERACTION") {
      blocks_count++;
      continue;
    }
    if (line.substr(0, 9) == "SUBSTRATE") {
      blocks_count++;
      continue;
    }
    if (line.substr(0, 8) == "ACTIVITY") {
      blocks_count++;
      continue;
    }
  }
}

auto readQSSPN(std::istream &s) {
  std::vector<std::tuple<std::string, counter_type, counter_type, uint8_t>>
      substances;
  metabolite_data met = metabolite_data();
  std::string line;
  while (s.good()) {
    std::getline(s, line);
    boost::trim(line);
    if (line == "SUBSTANCES") {
      substances = readSubstances(s);
    }
    if (line == "REACTIONS") {
      skipReactions(s);
    }
    if (line == "GSMN") {
      met = readMetaboliteData(s);
    }
  }
  std::vector<Metabolite> metabolite_vec;
  for (auto &el : met.metabolites) {
    met.targets_set.insert(el.goal());
  }
  return met;
}
/*!
 * Class for generate cartesian product
 *
 * @tparam T - type of iterated objects. Must support compare and size_t add
 */
template <typename T> class VectorIterator {
public:
  VectorIterator(const std::vector<std::vector<T>> &v,
                 std::pair<size_t, size_t> range)
      : iterate_values(v), range(range) {
    size_t begin = range.first;
    current = range.first;
    for (auto &el : iterate_values) {
      size_t num = begin % el.size();
      begin /= el.size();
      current_val.push_back(el[num]);
      iterator_vector.push_back(el.begin() + num);
    }
  };
  /*!
   * give current value of cartesian product
   * @return
   */
  std::vector<T> value() { return current_val; }
  /*!
   * generate next element of cartesian product and change state
   * @return
   */
  std::vector<T> next() {
    if (this->end())
      throw std::out_of_range(std::string(__FILE__) + ": " +
                              std::to_string(__LINE__));
    current++;
    auto val_it = current_val.begin();
    auto ite_it = iterator_vector.begin();
    auto vec_it = iterate_values.begin();
    bool br;
    for (size_t i = 0; i < current_val.size(); i++) {
      current_val[i] = *iterator_vector[i];
    }
    (*iterator_vector.begin())++;
    for (; ite_it != iterator_vector.end(); val_it++, ite_it++, vec_it++) {
      br = true;
      if (*ite_it == vec_it->end() && ite_it + 1 != iterator_vector.end()) {
        *ite_it = vec_it->begin();
        (*(ite_it + 1))++;
        br = false;
      }
      if (br)
        break;
    }
    return current_val;
  }
  /*!
   * check that is eny element of cartesian product that was not produced
   * @return
   */
  bool end() { return current >= range.second; }

    size_t length(){
      return range.second - range.first;
    }

private:
  std::vector<std::vector<T>> iterate_values;
  std::vector<typename std::vector<T>::const_iterator> iterator_vector;
  std::vector<T> current_val;
  std::pair<size_t, size_t> range;
  size_t current;
};
} // namespace

Metabolism::Metabolism(std::string qsspn_file_path,
                       std::string sfba_file_path) {
  auto file = std::ifstream(qsspn_file_path);
  // auto file2 = std::ifstream(sfba_file_path);
  auto met = readQSSPN(file);
  this->model_name_ = met.model_name;
  this->ext_tag_ = met.ext_tag;
  this->metabolites_ = met.metabolites;
  this->enzymes_ = met.enzymes;
  this->targets_set_ = met.targets_set;
  this->solver = new SFBA(sfba_file_path, this->ext_tag_, this->enzymes_);
  size_t len = 1;
  for (auto &el : enzymes_) {
    len *= el.get_threshold_values().size();
  }
  range_ = std::make_pair(0, len);
  this->solver->print_info();
}

void Metabolism::calculateRange(std::ostream &result_file, size_t begin, size_t end) const {
  size_t counter = 0;
  size_t variants = 1;
  std::vector<std::vector<counter_type>> enzyme_values;
  for (auto &el : enzymes_) {
    enzyme_values.push_back(el.get_threshold_values());
    variants *= enzyme_values.back().size();
  }
  /* std::cerr << "SIZES: ";
  for (auto &el : enzyme_values) {
    std::cerr << el.size() << " ";
  }*/
  std::vector<counter_type> dd;
  std::pair<size_t, size_t> range;
  if (end == 0){
    range = this->range_;
  } else {
    end = std::min(end, this->range_.second);
    range = std::make_pair(begin, end);
  }
  VectorIterator<counter_type> all_variant_iterator =
      VectorIterator<counter_type>(enzyme_values, range);
  // std::cerr << all_variant_iterator.length() << std::endl;
  if (!result_file.good()) {
    std::cerr << "Error" << std::endl;
    exit(-1);
  }
  while (!all_variant_iterator.end()) {
    dd = all_variant_iterator.next();
    auto constraint = this->solver->prepareProblemConstraints(dd);
    result_file << dd << " |";
    if (this->target != ""){
      double res = this->solver->optimize(this->target, constraint);
      result_file << " " << res << std::endl;
    } else {
      std::map<std::string, double> opt_res;
      for (auto &name : this->targets_set_) {
        opt_res[name] = this->solver->optimize(name, constraint);
      }
      for (auto &met : this->metabolites_) {
        result_file << " " << met(opt_res[met.goal()]);
      }
      result_file << std::endl;
    }
  }
}

SFBA::SFBA(std::string sfba_path, std::string ext_tag,
           const std::vector<Enzyme> &enzyme_vec) {
  auto is = std::ifstream(sfba_path);
  parse_lp_system(is, ext_tag);
  lp_solver_ = new PNFBA::LPSolverFacade(
      lp_system_row_names_, lp_system_col_names_, lp_system_row_index_,
      lp_system_col_index_, lp_system_i_, lp_system_j_, lp_system_val_);
  size_t i = 0;
  for (auto &enz : enzyme_vec) {
    auto reaction_list = enz.get_reaction_list();
    for (auto &reaction : reaction_list) {
      if (this->reaction_enzyme_map_.count(reaction) > 0) {
        throw FileFormatException("Double reaction use " + reaction + __FILE__ +
                                  ": " + std::to_string(__LINE__));
      }
      this->reaction_enzyme_map_.insert(
          std::make_pair(reaction, std::make_pair(i, &enz)));
    }
    i += 1;
  }
}

SFBA::ProblemConstraint
SFBA::prepareProblemConstraints(const std::vector<counter_type> marking) {
  SFBA::ProblemConstraint constraints_for_solver;
  for (auto bnd : default_constraints_) { // std::map<std::string,
                                          // std::pair<double, double> >
    int i = lp_system_col_index_.at(bnd.first);
    std::pair<double, double> constraint = bnd.second;
    if (this->reaction_enzyme_map_.count(bnd.first) > 0) {
      auto &pos_and_enzyme = this->reaction_enzyme_map_.at(bnd.first);
      constraint = (*pos_and_enzyme.second)(marking[pos_and_enzyme.first]);
    }
    constraints_for_solver.push_back(
        std::make_tuple(i, constraint.first, constraint.second));
  }
  return constraints_for_solver;
}

void SFBA::parse_lp_system(std::istream &metabolism_sfba,
                           const std::string &ext_tag) {
  // parsujemy plik sfba z metabolizmem, ktorego linie maja postac;
  // r0001 HC00683_c = HC00165_c + HC02119_c 0.0 1.0
  //# S-Adenosylmethioninamine(c) --> 5-Methylthioadenosine(c) +
  // Adenosylmethioninamine-potential(c)
  // najpierw jest nazwa reakcji/strumienia, potem reakcja, potem wiezy i
  // komentarz
  // substancje z nazwami konczacymi sie na ext_tag nie musza byc zbalansowane w
  // stanie stacjonarnym

  // zamiastr trojek trzymamy trzy wektory z dwoma indeksami oraz wartoscia
  // dummy values at index 0
  lp_system_i_.push_back(0); // to indeksy wierszy z nomenklatury GLPK (i z
  // macierzy metabolicznej), reprezentuja substancje
  // wystepujace w reakcjach
  lp_system_j_.push_back(0); // to indeksy kolumn z nomenklatury GLPK (i z
  // macierzy metabolicznej), to sa indeksy wierszy z
  // pliku (ktora linia z reakcja),
  // nasza optymalizacja sprawdza ze dla kazdego wiersza w sumie wychodzi 0
  lp_system_val_.push_back(0.0);
  int col_index = 1;
  int row_index = 1;
  while (metabolism_sfba.good()) {
    std::string line;
    std::getline(metabolism_sfba, line);
    boost::trim(line);
    std::stringstream line_stream(line);
    std::string stream_name;
    line_stream >> stream_name;
    if (boost::trim_copy(stream_name) == "") {
      std::cerr << "We shouldn't be here... (parse_lp_system)"; // w tym pliku
                                                                // nie ma
                                                                // pustych
                                                                // linii, GB -
                                                                // moze być na
                                                                // końcu
      continue;
    }
    boost::trim(stream_name);
    lp_system_col_names_.push_back(
        stream_name); // kolejne nazwy strumieni/reakcji
    lp_system_col_index_[stream_name] =
        col_index++; // kolejne indeksy dla nazw strumieni/reakcji

    // czyta lewa strone reakcji do '='
    std::string lhs;
    std::getline(line_stream, lhs, '=');
    parse_linear_expression(boost::trim_copy(lhs), col_index - 1, ext_tag, -1.0,
                            &row_index);

    // czyta prawa strone reakcji i domyslne bounds
    std::string rest;
    std::getline(line_stream, rest);
    // na rest jest prawa strona bez komentarza
    rest = rest.substr(0, rest.find_first_of('#'));
    // najpierw parsujemy do ostatniego plusa
    size_t last = rest.find_last_of('+');
    if (last != std::string::npos) {
      parse_linear_expression(rest.substr(0, last), col_index - 1, ext_tag, 1.0,
                              &row_index);
      rest = rest.substr(last + 1);
    }

    // potem parsujemy ostatni czlon i boundsy
    std::stringstream rhs_stream(rest);
    std::string token;
    rhs_stream >> token;
    double constant = 1.0;
    // tu moze byc liczba (bez +- i bez sesnu zeby bylo inf)
    if (std::isdigit(token.at(0))) {
      constant *= boost::lexical_cast<double>(token);
      rhs_stream >> token;
    }
    if (token.length() < ext_tag.length() ||
        token.substr(token.length() - ext_tag.length()) != ext_tag) {
      lp_system_j_.push_back(int(lp_system_col_index_.at(boost::trim_copy(
          stream_name)))); //!!!TODO??? tu wczesniej nie bylo trim dookola
      //! stream_name; mogloby byc w zamian col_index-1
      if (lp_system_row_index_.count(token) > 0) {
        lp_system_i_.push_back(int(lp_system_row_index_.at(token)));
      } else {
        lp_system_row_names_.push_back(token);
        lp_system_row_index_[token] = row_index++;
        lp_system_i_.push_back(int(lp_system_row_index_.at(token)));
      }
      lp_system_val_.push_back(constant);
    } //!!!TODO??? a czy cos nie powinnismy zrobic jak substancja ma ext_tag?
    std::pair<double, double> constraints;
    rhs_stream >> constraints.first >> constraints.second;
    default_constraints_[boost::trim_copy(stream_name)] =
        constraints; //!!!TODO??? tu wczesniej nie bylo trim dookola stream_name
  }
  //!!!TODO??? przydaloby sie wypisac wszystkie lp_system_row_names_ oraz
  //! lp_system_col_names_ zeby sprawdzic czy sa poprawnie trimowane
}

void SFBA::parse_linear_expression(const std::string &expression, const int j,
                                   const std::string &ext_tag, double sign,
                                   int *index) {

  // czytamy lewa lub prawa strone reakcji (jak prawa to bez ostatniego
  // skladnika)
  std::string token;
  std::stringstream stream(expression);
  std::getline(stream, token, '+');
  while (!stream.fail()) {
    std::stringstream atom(boost::trim_copy(token));
    std::string word;
    atom >> word;
    double constant = sign;
    if (std::isdigit(word.at(0))) {
      constant *=
          boost::lexical_cast<double>(word); // TODO zamienic na parsowanie ze
                                             // standardowych bibliotek, bo tu
                                             // nie moze byc +inf bo isdigit !!!
      atom >> word;
    }
    if (word.length() >= ext_tag.length() &&
        word.substr(word.length() - ext_tag.length()) == ext_tag) {
      std::getline(stream, token, '+');
      //!!!TODO??? a czy cos nie powinnismy zrobic jak substancja ma ext_tag?
      // trzeba by to zapamietywac, bo moze byc cel optymalizacji
      continue;
    }
    lp_system_j_.push_back(j);
    if (lp_system_row_index_.count(word) > 0) {
      lp_system_i_.push_back(int(lp_system_row_index_.at(word)));
    } else {
      lp_system_row_names_.push_back(word);
      lp_system_row_index_[word] = (*index)++;
      lp_system_i_.push_back(int(lp_system_row_index_.at(word)));
    }
    lp_system_val_.push_back(constant);
    std::getline(stream, token, '+');
  }
}
