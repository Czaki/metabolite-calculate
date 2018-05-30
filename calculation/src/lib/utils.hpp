//
// Created by Grzegorz Bokota on 04.06.2017.
//

#ifndef METABOLITE_UTILS_HXX
#define METABOLITE_UTILS_HXX

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <istream>
#include <ostream>
#include <vector>

typedef uint32_t counter_type;

namespace utils {

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

  size_t get_pos(std::vector<T> vec) {
    size_t res = 0;
    size_t pow = 1;
    for (size_t j = 0; j < iterate_values.size(); ++j) {
      T val = vec[j];
      auto pos =
          std::find(iterate_values[j].begin(), iterate_values[j].end(), val);
      assert(pos < iterate_values[j].end());
      res += (pos - iterate_values[j].begin()) * pow;
      pow *= iterate_values[j].size();
    }

    return res;
  }

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

  size_t length() { return range.second - range.first; }

private:
  std::vector<std::vector<T>> iterate_values;
  std::vector<typename std::vector<T>::const_iterator> iterator_vector;
  std::vector<T> current_val;
  std::pair<size_t, size_t> range;
  size_t current;
};

template <typename T, typename K> class MetabolismResult {
public:
  std::vector<T> marking;
  K result;
  friend std::ostream &operator<<(std::ostream &stream,
                                  const MetabolismResult &res) {
    for (const T &val : res.marking) {
      stream << val << " ";
    }
    stream << "| " << res.result;
    return stream;
  }
  friend std::istream &operator>>(std::istream &stream, MetabolismResult &res) {
    res.marking.clear();
    std::string tmp;
    stream >> tmp;
    while (tmp != "|") {
      res.marking.push_back(boost::lexical_cast<T>(tmp));
      stream >> tmp;
    }
    stream >> res.result;
    return stream;
  }
};
} // namespace utils

#endif // METABOLITE_UTILS_HXX
