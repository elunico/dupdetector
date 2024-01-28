#include <string>
#include <unordered_set>
#include <vector>

#include "argparse.hpp"

struct duplicate_remover {
  std::unordered_set<std::string> seen{};
  bool quiet;

  void operator()(std::string const &hash, std::vector<std::string> &filenames);
};

struct duplicate_printer {
  bool doSort;
  std::unique_ptr<comparator> comparator;

  void operator()(std::string const &hash,
                  std::vector<std::string> &filenames) const;
};
