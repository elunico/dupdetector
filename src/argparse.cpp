#include "argparse.hpp"

#include <filesystem>
#include <stdexcept>
#include "duplicate_actions.hpp"

namespace tom::dupdetect {

void usage() {
  printf("./main DIRECTORY\n");
  printf(
      "  this is the default action to recursively scan DIRECTORY for"
      " duplicates and print out all the duplicates at the end. For more "
      "options, use the flags below\n\n");
  printf("./main -d DIRECTORY [ -r | -g DUP_DIR [ -n | -o ] ] [ -q ]\n");
  printf("       -d DIRECTORY the directory to scan for duplicates\n");
  printf(
      "       if -r is specified all but one of the duplicates found are "
      "DELETED. \n");
  printf(
      "       if -g DUP_DIR is specified, all but one duplicate files will be "
      "moved into the folder DUP_DIR. This folder MUST be a subfolder of "
      "DIRECTORY\n");
  printf(
      "         if either -r or -g is used, then the order in which files are "
      "organized can be affected. The LAST file in each list survives\n");
  printf(
      "         -n sort duplicates by newest file write time first, thus "
      "saving "
      "the oldest file\n");
  printf(
      "         -o sort duplicates by oldest file write time first, thus "
      "saving "
      "the newest file\n");
  printf("       -q run in quiet mode\n");
}

typename duplicate_printer::comparator_type
get_comparator(std::optional<typename arguments::comparison_method> method) {
  if (!method.has_value())
    return (typename duplicate_printer::comparator_type) nullptr;
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
      return nullptr;
    }
  }
}

arguments parse_complex_args(int argc, char* const argv[]) {
  arguments retval{};
  for (;;) {
    switch (getopt(argc, argv, "qonrd:g:")) {
      case 'g':
        retval.target_dir = optarg;
        continue;

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
  if (retval.doRemove && retval.target_dir.has_value()) {
    throw std::invalid_argument(
        "Specify either -r for remove or -g for renaming files but not both");
  }
  return retval;

die:
  throw std::invalid_argument("Invalid CLI args");
}

arguments get_args(int argc, char *const argv[]) {
  arguments args{};
  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'h') {
    usage();
    exit(1);
  } else if (argc == 2 && std::string(argv[1]) != "-h") {
    if (std::filesystem::is_directory(argv[1])) {
      args.directory = argv[1];
      args.doRemove = false;
      args.method = std::nullopt;
      args.quiet = false;
    }
  } else {
    args = parse_complex_args(argc, argv);
  }
  return args;
}

}  // namespace tom::dupdetect
