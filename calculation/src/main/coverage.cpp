//
// Created by Grzegorz Bokota on 04.06.2017.
//
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../lib/Metabolism.hpp"
#include "../lib/utils.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

#define REPORT_FILE "report.txt"
#define LOST_FILE "lost.txt"
#define COMPRESED_FILE "compressed.txt"

po::variables_map parse_options(int argc, char *argv[]) {
  po::options_description options("Allowed options");
  options.add_options()("qsspn_file", po::value<std::string>(),
                        "path to qsspn file")(
      "sfba_file", po::value<std::string>(),
      "path to sfba file")("output_dir", po::value<std::string>(),
                           "path to directory to place result")(
      "directory", po::value<std::vector<std::string>>(),
      "directories to search")("depth", po::value<int>(), "depth of search")(
      "prefix", po::value<std::string>()->default_value("part"),
      "prefix of result file ");

  po::positional_options_description p;
  p.add("qsspn_file", 1);
  p.add("sfba_file", 1);
  p.add("output_dir", 1);
  p.add("depth", 1);
  p.add("directory", -1);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, (const char **)argv)
                  .options(options)
                  .positional(p)
                  .run(),
              vm);
    po::notify(vm);
  } catch (std::exception const &e) {
    std::cout << "Argument error: ";
    std::cout << e.what() << std::endl;
    std::cout << options << std::endl;
    exit(2);
  }
  if (vm.count("help")) {
    std::cout << options << std::endl;
    exit(1);
  }
  if (vm.count("directory") == 0 || vm.count("depth") == 0) {
    std::cout << "Wrong arguments" << std::endl;
    std::cout << options << std::endl;
    exit(1);
  }
  if (vm.count("output_dir") == 0) {
    std::cout << "output_dir lost" << std::endl;
    std::cout << options << std::endl;
    exit(1);
  } else {
    fs::path dir_path(vm.at("output_dir").as<std::string>());
    if (fs::exists(dir_path)) {
      if (fs::is_directory(dir_path)) {
        std::vector<std::string> out_files = {REPORT_FILE, LOST_FILE,
                                              COMPRESED_FILE};
        bool error = false;
        for (std::string &name : out_files) {
          fs::path cp = dir_path;
          cp.append(name);
          if (fs::exists(cp)) {
            std::cerr << "[WARNING] file " << cp << " already exists"
                      << std::endl;
            error = true;
          }
        }
        /*if (error){
          exit(1);
        }*/
      }
    } else {
      fs::create_directories(dir_path);
    }
  }
  return vm;
}

int main(int argc, char *argv[]) {
  po::variables_map options = parse_options(argc, argv);
  std::vector<fs::path> to_check;
  for (auto &dir_path :
       options.at("directory").as<std::vector<std::string>>()) {
    fs::path path(dir_path);
    if (!fs::is_directory(path)) {
      std::cerr << dir_path << " is not directory" << std::endl;
    } else {
      to_check.push_back(path);
    }
  }
  int depth = options.at("depth").as<int>();
  while (depth > 0) {
    std::vector<fs::path> current_check = to_check;
    to_check.clear();
    for (auto &dir : current_check) {
      for (fs::directory_entry &file : fs::directory_iterator(dir)) {
        if (fs::is_directory(file.path())) {
          to_check.push_back(file.path());
        }
      }
    }
    depth--;
  }
  const std::string prefix = options.at("prefix").as<std::string>();

  std::map<std::pair<size_t, size_t>, std::string> coverage_map;
  Metabolism met(options.at("qsspn_file").as<std::string>(),
                 options.at("sfba_file").as<std::string>());
  auto vec = met.get_vector();
  size_t range_begin = 0;
  size_t range_end = 0;
  std::string range_value = "";

  for (auto &dir : to_check) {
    for (fs::directory_entry &file : fs::directory_iterator(dir)) {
      if (fs::is_regular_file(file.path())) {
        std::string name = file.path().filename().string();
        std::string part = name.substr(0, prefix.length());
        if (prefix != part) {
          continue;
        }
        std::ifstream result_file(file.path().string());
        utils::MetabolismResult<counter_type, std::string> res;
        while (!result_file.eof()) {
          std::string line;
          std::getline(result_file, line);
          if (line == "")
            continue;
          std::stringstream ss(line);
          try {
            ss >> res;
          } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::cerr << file << std::endl;
            std::cerr << line << std::endl;
            break;
          }
          std::vector<std::pair<counter_type, counter_type>> to_remove;
          size_t pos = vec.get_pos(res.marking);
          if (res.result != range_value || range_end != pos) {
            if (coverage_map.size() != 0) {
              auto range_it = coverage_map.lower_bound(
                  std::make_pair(range_begin, range_end));
              if (range_it != coverage_map.end()) {
                if (range_it->second == range_value) {
                  if (range_it->first.first <= range_end) {
                    if (range_it->first.first < range_end) {
                      std::cerr << "[WARNING] overleap {end}" << range_end
                                << " " << file << std::endl;
                    }
                    to_remove.push_back(range_it->first);
                    range_end = range_it->first.second;
                  }
                }
              }
              if (range_it != coverage_map.begin()) {
                auto prev_it = range_it;
                prev_it--;
                if (prev_it->second == range_value) {
                  if (prev_it->first.second >= range_begin) {
                    if (prev_it->first.second > range_begin) {
                      std::cerr << "[WARNING] overleap {begin}" << range_begin
                                << " " << file << std::endl;
                    }
                    to_remove.push_back(prev_it->first);
                    range_begin = prev_it->first.first;
                  }
                }
              }
              for (auto &val : to_remove) {
                coverage_map.erase(val);
              }
            }
            if (range_begin != range_end)
              coverage_map.insert(std::make_pair(
                  std::make_pair(range_begin, range_end), range_value));
            range_begin = pos;
            range_end = pos + 1;
            range_value = res.result;
          } else {
            range_end++;
          }
        }
        result_file.close();
      }
    }
  }
  coverage_map.insert(
      std::make_pair(std::make_pair(range_begin, range_end), range_value));

  auto total_range = met.get_range();

  fs::path dir_path(options.at("output_dir").as<std::string>());
  fs::path cp;
  cp = dir_path;
  cp.append(REPORT_FILE);
  std::ofstream report_file(cp.string());
  cp = dir_path;
  cp.append(LOST_FILE);
  std::ofstream lost_file(cp.string());
  cp = dir_path;
  cp.append(COMPRESED_FILE);
  std::ofstream compressed_file(cp.string());

  size_t lost = 0;

  std::pair<size_t, size_t> prev(total_range.first, total_range.first);
  for (auto &it : coverage_map) {
    if (it.first.first != prev.second) {
      lost_file << "(" << prev.second << "," << it.first.first << ")"
                << std::endl;
      lost += it.first.first - prev.second;
    }
    compressed_file << "(" << it.first.first << "," << it.first.second << ") "
                    << it.second << std::endl;
    prev = it.first;
  }
  compressed_file.close();
  auto last = coverage_map.rbegin();
  if (coverage_map.size() > 0) {
    size_t lost_end = total_range.second - last->first.second;
    std::cerr << lost_end << std::endl;
    lost += lost_end;
    if (lost_end > 0) {
      lost_file << "(" << last->first.second << "," << total_range.second << ")"
                << std::endl;
    }
  } else {
    lost_file << "(" << total_range.first << "," << total_range.second << ")"
              << std::endl;
  }
  lost_file.close();
  report_file << "Total ranges: " << coverage_map.size() << std::endl;
  report_file << "Percent lost: "
              << float(lost) / (total_range.second - total_range.first) * 100
              << "%" << std::endl;
  return 0;
}
