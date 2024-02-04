#ifndef ARGPARSE_HPP
#define ARGPARSE_HPP

#include <unistd.h>

#include <memory>
#include <optional>
#include <string>

namespace tom::dupdetect {

struct arguments {
  enum class comparison_method { OLDEST, NEWEST, RANDOM };

  std::optional<comparison_method> method = std::nullopt;
  bool doRemove{};
  bool quiet{};
  std::optional<std::string> directory = std::nullopt;
};

arguments parse_args(int argc, char* const argv[]);

auto get_comparator(std::optional<arguments::comparison_method> method)
    -> bool (*)(std::string const&, std::string const&);

}  // namespace tom::dupdetect
#endif
