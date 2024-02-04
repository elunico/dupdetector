#include "print_util.hpp"

#include <mutex>
#include <sstream>
#include <utility>

namespace tom::utils {
times::times(std::string s, unsigned long long count)
    : count(count), data(std::move(s)) {}

std::ostream& operator<<(std::ostream& os, times const& time) {
  for (decltype(time.count) i = 0; i < time.count; i++) {
    os << time.data;
  }
  return os;
}

static std::mutex outmut{};

void print_hashed_message(std::filesystem::directory_entry const& entry,
                          unsigned long long count,
                          std::ostream& os) {
  std::unique_lock l(outmut);
  static unsigned long long cols = strtoull(getenv("COLUMNS"), nullptr, 10);
  std::stringstream s;
  s << "[" << count << "] "
    << "Hashing " << entry.path();
  std::string msg = s.str();
  if (msg.size() >= cols) {
    msg = msg.substr(0, cols - 1 - 3) + "...";
  }
  os << msg << times(" ", cols - msg.length()) << times("\x08", cols);
}

std::string times::operator*() const {
  std::stringstream s;
  s << *this;
  return s.str();
}
}  // namespace tom::utils
