#ifndef DUPLICATE_ACTIONS_HPP
#define DUPLICATE_ACTIONS_HPP

#include <string>
#include <unordered_set>
#include <vector>

#include "argparse.hpp"
namespace tom::dupdetect {

struct duplicate_remover {
  std::unordered_set<std::string> seen{};
  bool quiet;

  void operator()(std::string const& hash, std::vector<std::string>& filenames);
};

struct duplicate_printer {
  using comparator_type = bool (*)(std::string const&, std::string const&);

  bool doSort;
  comparator_type comparator;

  void operator()(std::string const& hash,
                  std::vector<std::string>& filenames) const;
};

}  // namespace tom::dupdetect

#endif
