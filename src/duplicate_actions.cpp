#include "duplicate_actions.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>

namespace tom::dupdetect {

void duplicate_remover::operator()(std::string const& hash,
                                   std::vector<std::string>& filenames) {
  if (seen.contains(hash))
    return;
  seen.insert(hash);
  auto const oldSize = filenames.size();
  auto const survivor = filenames[filenames.size() - 1];
  if (!quiet) {
    std::cerr << hash << ": designated survivor is " << survivor << std::endl;
  }
  filenames.erase(filenames.end() - 1);
  auto const newSize = filenames.size();
  assert(oldSize == (newSize + 1));
  for (auto const& filename : filenames) {
    if (!quiet) {
      std::cerr << "\t [-]: removing " << filename << std::endl;
    }
    std::filesystem::remove(filename);
  }
}

void duplicate_printer::operator()(std::string const& hash,
                                   std::vector<std::string>& filenames) const {
  long dupcount = 1;
  std::cout << "Duplicates of " << hash << std::endl;
  if (doSort)
    std::sort(filenames.begin(), filenames.end(), comparator);
  for (auto& filename : filenames) {
    std::cout << "\t" << dupcount++ << " " << filename << std::endl;
  }
}
}  // namespace tom::dupdetect
