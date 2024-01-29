#include "print_util.hpp"

#include <sstream>
#include <utility>

times::times(std::string s, unsigned long long count)
    : count(count), data(std::move(s)) {}

std::ostream& operator<<(std::ostream& os, times const& time) {
  for (decltype(time.count) i = 0; i < time.count; i++) {
    os << time.data;
  }
  return os;
}

void print_hashed_message(std::filesystem::directory_entry const& entry,
                          unsigned long long count) {
  static unsigned long long cols = strtoull(getenv("COLUMNS"), nullptr, 10);
  std::stringstream s;
  s << "[" << count << "] "
    << "Hashing " << entry.path() << "...";
  std::string msg = s.str();
  std::cerr << msg << times(" ", cols - msg.length()) << times("\x08", cols);
}

std::string times::operator*() const {
  std::stringstream s;
  s << *this;
  return s.str();
}
