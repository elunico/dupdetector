#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>

#include "argparse.hpp"
#include "duplicate_actions.hpp"
#include "print_util.hpp"
#include "sha256util.hpp"

struct hashed_directory {
  std::unordered_map<std::string, std::vector<std::string>> duplicates;
  unsigned long long count;
};

hashed_directory get_duplicate_contents(std::string const& directory) {
  std::vector<std::filesystem::directory_entry> work{};
  std::vector<std::tuple<std::string, std::filesystem::directory_entry>>
      results{};

  std::size_t count = 0;
  std::unordered_map<std::string, std::vector<std::string>> record{};

  std::thread mapper{[&record, &results, &count]() {
    while (count == 0 && !results.empty()) {
      auto [hash, entry] = results[results.size() - 1];
      record[hash].push_back(entry.path());
      results.pop_back();
    }
  }};
  std::vector<std::thread> threads{};
  for (auto const& entry :
       std::filesystem::recursive_directory_iterator(directory)) {
    // record[hashed].push_back(entry.path());
    // print_hashed_message(entry, count);
    work.push_back(entry);
  }

  for (int i = 0; i < 16; i++) {
    std::thread t([&work, &count, &results]() {
      while (!work.empty()) {
        auto entry = work[work.size() - 1];
        auto hashed = SHA256Hash::ofFile(entry.path()).hex();
        results.push_back(std::make_tuple(hashed, entry));
        print_hashed_message(entry, count);
        count++;
        work.pop_back();
      }
    });
    threads.push_back(std::move(t));
  }

  for (int i = 0; i < 16; i++)
    threads[i].join();

  return {record, count};
}

void process_duplicates(hashed_directory& directory, auto operation) {
  for (auto& [hash, filenames] : directory.duplicates) {
    if (filenames.size() > 1) {
      operation(hash, filenames);
    }
  }
}

int main(int argc, char* const argv[]) {
  arguments args = parse_args(argc, argv);
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
    process_duplicates(hdir, duplicate_printer{comp != nullptr, comp});
  }

  return 0;
}
