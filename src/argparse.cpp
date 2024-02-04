#include "argparse.hpp"

#include <filesystem>
#include "duplicate_actions.hpp"

namespace tom::dupdetect {

void usage() {
  printf("./main -d DIRECTORY [ -n | -o ] [ -r ] [ -q ]\n");
  printf("       -d DIRECTORY the directory to scan for duplicates\n");
  printf("       -n sort duplicates by newest file write time first\n");
  printf("       -o sort duplicates by oldest file write time first\n");
  printf("       -q run in quiet mode\n");
  printf(
      "       -r DELETE all but the first of all duplicates of each "
      "file. Can "
      "be combined with -n and -o.\n");
}

duplicate_printer::comparator_type get_comparator(
    std::optional<arguments::comparison_method> method) {
  if (!method.has_value())
    return (duplicate_printer::comparator_type) nullptr;
  switch (*method) {
    case arguments::comparison_method::NEWEST: {
      return [](std::string const& s, std::string const& r) -> bool {
        return std::filesystem::last_write_time(std::filesystem::path{s}) <
               std::filesystem::last_write_time(std::filesystem::path{r});
      };
    }
    case arguments::comparison_method::OLDEST: {
      return [](std::string const& s, std::string const& r) -> bool {
        return std::filesystem::last_write_time(std::filesystem::path{s}) <
               std::filesystem::last_write_time(std::filesystem::path{r});
      };
    }
    case arguments::comparison_method::RANDOM:
    default: {
      return [](std::string const& s, std::string const& r) -> bool {
        return s < r;
      };
    }
  }
}

arguments parse_args(int argc, char* const argv[]) {
  arguments retval{};
  for (;;) {
    switch (getopt(argc, argv, "qonrd:")) {
      case 'd':
        retval.directory = optarg;
        continue;

      case 'o':
        if (retval.method.has_value()) {
          throw std::invalid_argument("Set -o or -n but not both!");
        }
        retval.method = arguments::comparison_method::OLDEST;
        continue;

      case 'n':
        if (retval.method.has_value()) {
          throw std::invalid_argument("Set -o or -n but not both!");
        }
        retval.method = arguments::comparison_method::NEWEST;
        continue;
      case 'r':
        retval.doRemove = true;
        continue;
      case 'q':
        retval.quiet = true;
        continue;

      case 'h':
        usage();
        break;

      case '?':
      default:
        usage();
        goto die;

      case -1:
        break;
    }

    break;
  }
  if (!std::filesystem::is_directory(*retval.directory)) {
    throw std::invalid_argument("Specify path as first argument");
  }
  return retval;

die:
  throw std::invalid_argument("Invalid CLI args");
}

}  // namespace tom::dupdetect
