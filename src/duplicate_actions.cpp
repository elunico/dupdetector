#include "duplicate_actions.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

void duplicate_remover::operator()(std::string const &hash,
                                   std::vector<std::string> &filenames) {
  if (seen.contains(hash)) return;
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

void duplicate_printer::operator()(std::string const &hash,
                                   std::vector<std::string> &filenames) const {
  long dupcount = 1;
  std::cout << "Duplicates of " << hash << std::endl;
  if (doSort)
    std::sort(filenames.begin(), filenames.end(),
              [this](auto a, auto b) { return (*(this->comparator))(a, b); });
  for (auto &filename : filenames) {
    std::cout << "\t" << dupcount++ << " " << filename << std::endl;
  }
}
