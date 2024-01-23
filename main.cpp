#include <functional>
#include <iostream>
#include "sha256util.hpp"
#include <map>
#include <filesystem>

using ull = unsigned long long ;

struct times {
  ull count; 
  std::string data; 

  times(std::string const& s, ull count): data(s), count(count) {
    
  }
};

std::ostream& operator<<(std::ostream& os, times const& time) {
  for (decltype(time.count) i = 0; i < time.count; i++) {
    os << time.data;     
  }
  return os; 
}

int main(int argc, char const *argv[]) {
  using namespace std::literals::string_literals;
  if (argc < 2) {
    std::cerr << "Specify path to scan" << std::endl;
    return 1;
  }

  ull cols = strtoull(getenv("COLUMNS"), NULL, 10);

  std::size_t count = 0;
  std::unordered_map<std::string, std::vector<std::string>> record{};
  for (const auto & entry : std::filesystem::recursive_directory_iterator(argv[1])) {
    auto hashed = SHA256Hash::ofFile(entry.path()).hex();
    record[hashed].push_back(entry.path());
    std::stringstream s; 
    s << "[" << count++ << "] " << "Hashing " << entry.path() << "...";
    std::string msg = s.str();
    std::cerr << msg << times(" ", cols - msg.length()) << times("\x08", cols);
  }

  std::stringstream s;
  s << "Scanned " << count-1 << " files";
  std::string msg = s.str();
  std::cerr << msg << times(" ", count - msg.length()) << std::endl;
  for (auto const& [hash, filenames]: record) {
    if (filenames.size() > 1) {
      long dupcount = 1;
      std::cout << "Duplicates of " << hash << std::endl;
      for (auto const& filename : filenames) {
        std::cout << "\t" << dupcount++ << " " << filename << std::endl;
      }
    }
  }

  return 0; 
}
