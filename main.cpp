#include <optional>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "argparse.hpp"
#include "duplicate_actions.hpp"
#include "print_util.hpp"
#include "sha256util.hpp"

struct hashed_directory {
  std::unordered_map<std::string, std::vector<std::string>> duplicates;
  unsigned long long count;
};

hashed_directory get_duplicate_contents(std::string const &directory) {
  std::size_t count = 0;
  std::unordered_map<std::string, std::vector<std::string>> record{};
  for (auto const &entry :
       std::filesystem::recursive_directory_iterator(directory)) {
    auto hashed = SHA256Hash::ofFile(entry.path()).hex();
    record[hashed].push_back(entry.path());
    print_hashed_message(entry, count);
    count++;
  }
  return {record, count};
}

void process_duplicates(hashed_directory &directory, auto operation) {
  for (auto &[hash, filenames] : directory.duplicates) {
    if (filenames.size() > 1) {
      operation(hash, filenames);
    }
  }
}

int main(int argc, char *const argv[]) {
  arguments args = parse_args(argc, argv);
  // std::string q;
  // std::cin >> q;
  // arguments args{};
  // args.directory =
  //     std::make_optional("/Users/thomaspovinelli/Desktop/screenshots");
  // args.doRemove = false;
  // args.quiet = false;
  // args.method = arguments::comparison_method::NEWEST;

  // using comparator_type = bool (*)(std::string const &, std::string const &);

  auto comp = get_comparator(args.method);

  auto hdir = get_duplicate_contents(*args.directory);
  auto [record, count] = hdir;

  std::stringstream s;
  s << "Scanned " << count - 1 << " files";
  std::string msg = s.str();
  std::cout << msg << times(" ", count - msg.length() - 1) << std::endl;

  if (args.doRemove) {
    process_duplicates(hdir, duplicate_remover{});
  } else {
    process_duplicates(hdir,
                       duplicate_printer{comp != nullptr, std::move(comp)});
  }

  return 0;
}
