#include <iostream>
#include "sha256util.hpp"
#include <map>
#include <filesystem>

void clearLine(std::ostream& os, unsigned long long cols) {
    for (int i = 0; i < cols; i++)
      os << "\x08";
}

void multiWrite(std::ostream& os, char c, unsigned long long amt) {
  for (int i = 0; i < amt; i++)
    os << c; 
}

template<typename streamable>
std::string operator*(streamable const& s, unsigned long long count) {
  std::stringstream stream;  
  for (decltype(count) i = 0; i < count; i++) {
    stream << s; 
  }
  return stream.str(); 
}

int main(int argc, char const *argv[]) {
  using namespace std::literals::string_literals;
  if (argc < 2) {
    std::cerr << "Specify path to scan" << std::endl;
    return 1;
  }

  unsigned long long cols = strtoull(getenv("COLUMNS"), NULL, 10);

  std::size_t count = 0;
  std::unordered_map<std::string, std::vector<std::string>> record{};
  for (const auto & entry : std::filesystem::recursive_directory_iterator(argv[1])) {
    auto hashed = SHA256Hash::ofFile(entry.path()).hex();
    record[hashed].push_back(entry.path());
    std::stringstream s; 
    s << "[" << count++ << "] " << "Hashing " << entry.path() << "...";
    std::string msg = s.str();
    std::cerr << msg; 
    std::cerr << (" "s * (cols - msg.length()));
    std::cerr << ("\x08"s * cols);
  }

  std::stringstream s;
  s << "Scanned " << count-1 << " files";
  std::string msg = s.str();
  std::cerr << msg << (" "s * (count - msg.length()));
  std::cerr << std::endl;
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
