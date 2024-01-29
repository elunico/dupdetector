#include <filesystem>
#include <iostream>
#include <string>

struct times {
  unsigned long long count;
  std::string data;

  times(std::string s, unsigned long long count);

  std::string operator*() const;
};

std::ostream& operator<<(std::ostream& os, times const& time);

void print_hashed_message(std::filesystem::directory_entry const& entry,
                          unsigned long long count);
