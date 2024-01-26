#include "sha256util.hpp"
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

using ull = unsigned long long;

struct times {
  ull count;
  std::string data;

  times(std::string const &s, ull count) : data(s), count(count) {}
};

struct hashed_directory {
  std::unordered_map<std::string, std::vector<std::string>> duplicates;
  ull count;
};

std::ostream &operator<<(std::ostream &os, times const &time) {
  for (decltype(time.count) i = 0; i < time.count; i++) {
    os << time.data;
  }
  return os;
}

bool sfcomparator(std::string const &s, std::string const &r) {
  return std::filesystem::last_write_time(std::filesystem::path{s}) <
         std::filesystem::last_write_time(std::filesystem::path{r});
}

bool srcomparator(std::string const &s, std::string const &r) {
  return !sfcomparator(s, r) || s != r;
}

bool sacomparator(std::string const &s, std::string const &r) { return s < r; }

void print_hashed_message(std::filesystem::directory_entry const &entry,
                          ull count) {
  static ull cols = strtoull(getenv("COLUMNS"), NULL, 10);
  std::stringstream s;
  s << "[" << count << "] "
    << "Hashing " << entry.path() << "...";
  std::string msg = s.str();
  std::cerr << msg << times(" ", cols - msg.length()) << times("\x08", cols);
}

hashed_directory get_duplicate_contents(std::string const &directory) {
  std::size_t count = 0;
  std::unordered_map<std::string, std::vector<std::string>> record{};
  for (auto const &entry :
       std::filesystem::recursive_directory_iterator(directory)) {
    auto hashed = SHA256Hash::ofFile(entry.path()).hex();
    record[hashed].push_back(entry.path());
    print_hashed_message(entry, count);
    count++;
  }
  return {record, count};
}

void process_duplicates(hashed_directory &directory, auto operation) {
  for (auto &[hash, filenames] : directory.duplicates) {
    if (filenames.size() > 1) {
      operation(hash, filenames);
    }
  }
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cerr << "Specify path to scan" << std::endl;
    return 1;
  }

  if (!std::filesystem::is_directory(argv[1])) {
    std::cerr << "Specify path as first argument" << std::endl;
    return 2;
  }

  bool (*comparator)(std::string const &, std::string const &) = sacomparator;
  if (argc == 3) {
    std::string opt = argv[2];
    if (opt == "-o") {
      comparator = sfcomparator;
    } else if (opt == "-n") {
      comparator = srcomparator;
    } else {
      comparator = sacomparator;
    }
  }

  // std::size_t count = 0;
  auto hdir = get_duplicate_contents(argv[1]);
  auto [record, count] = hdir;

  std::stringstream s;
  s << "Scanned " << count - 1 << " files";
  std::string msg = s.str();
  std::cerr << msg << times(" ", count - msg.length()) << std::endl;

  process_duplicates(
      hdir, [argc, comparator](std::string const &hash,
                               std::vector<std::string> &filenames) {
        long dupcount = 1;
        std::cout << "Duplicates of " << hash << std::endl;
        if (argc == 3)
          std::sort(filenames.begin(), filenames.end(), comparator);
        for (auto &filename : filenames) {
          std::cout << "\t" << dupcount++ << " " << filename << std::endl;
        }
      });

  return 0;
}
