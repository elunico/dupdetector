#include "sha256util.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

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

struct duplicate_remover {
  std::unordered_set<std::string> seen{};
  bool quiet;

  void operator()(std::string const &hash,
                  std::vector<std::string> &filenames) {
    if (seen.contains(hash))
      return;
    seen.insert(hash);
    auto oldSize = filenames.size();
    auto survivor = filenames[filenames.size() - 1];
    if (!quiet) {
      std::cerr << hash << ": designated survivor is " << survivor << std::endl;
    }
    filenames.erase(filenames.end() - 1);
    auto newSize = filenames.size();
    assert(oldSize == (newSize + 1));
    for (int i = 0; i < filenames.size(); i++) {
      if (!quiet) {
        std::cerr << "\t [-]: removing " << filenames[i] << std::endl;
      }
      std::filesystem::remove(filenames[i]);
    }
  }
};

struct duplicate_printer {
  bool doSort;
  std::function<bool(std::string &, std::string &)> comparator;

  void operator()(std::string const &hash,
                  std::vector<std::string> &filenames) {
    long dupcount = 1;
    std::cout << "Duplicates of " << hash << std::endl;
    if (doSort)
      std::sort(filenames.begin(), filenames.end(), comparator);
    for (auto &filename : filenames) {
      std::cout << "\t" << dupcount++ << " " << filename << std::endl;
    }
  }
};

struct arguments {
  enum class comparison_method { OLDEST, NEWEST, RANDOM };

  std::optional<comparison_method> method = std::nullopt;
  bool doRemove;
  bool quiet;
  std::optional<std::string> directory = std::nullopt;
};

arguments parse_args(int argc, char *const argv[]) {
  arguments retval{};
  for (;;) {
    switch (getopt(argc, argv, "qonrd:")) {
    case 'd':
      retval.directory = optarg;
      continue;

    case 'o':
      if (retval.method.has_value()) {
        throw std::invalid_argument("Set -o or -n but not both!");
      }
      retval.method = arguments::comparison_method::OLDEST;
      continue;

    case 'n':
      if (retval.method.has_value()) {
        throw std::invalid_argument("Set -o or -n but not both!");
      }
      retval.method = arguments::comparison_method::NEWEST;
      continue;
    case 'r':
      retval.doRemove = true;
      continue;
    case 'q':
      retval.quiet = true;
      continue;

    case '?':
    case 'h':
    default:
      printf("Help/Usage Example\n");
      break;

    case -1:
      break;
    }

    break;
  }
  if (!retval.directory.has_value()) {
    throw std::invalid_argument("Must provide directory after -d");
  }
  if (!std::filesystem::is_directory(*retval.directory)) {
    throw std::invalid_argument("Specify path as first argument");
  }
  return retval;
}

auto get_comparator(std::optional<arguments::comparison_method> method) {
  bool (*comparator)(std::string const &, std::string const &) = nullptr;
  if (!method.has_value())
    return (decltype(comparator))nullptr;
  switch (*method) {
  case arguments::comparison_method::NEWEST:
    comparator = srcomparator;
  case arguments::comparison_method::OLDEST:
    comparator = sfcomparator;
  case arguments::comparison_method::RANDOM:
  default:
    comparator = sacomparator;
  }
  return comparator;
}

int main(int argc, char *const argv[]) {
  arguments args = parse_args(argc, argv);

  bool (*comparator)(std::string const &, std::string const &) =
      get_comparator(args.method);

  // std::size_t count = 0;
  auto hdir = get_duplicate_contents(*args.directory);
  auto [record, count] = hdir;

  std::stringstream s;
  s << "Scanned " << count - 1 << " files";
  std::string msg = s.str();
  std::cerr << msg << times(" ", count - msg.length()) << std::endl;

  if (args.doRemove) {
    process_duplicates(hdir, duplicate_remover{});
  } else {
    process_duplicates(hdir,
                       duplicate_printer{comparator != nullptr, comparator});
  }

  return 0;
}
