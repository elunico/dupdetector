#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
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

struct preprocessed_result {
  std::string hash;
  std::filesystem::directory_entry entry;
};

hashed_directory get_duplicate_contents(std::string const& directory) {
  std::vector<std::filesystem::directory_entry> work{};
  std::vector<preprocessed_result> results{};

  std::size_t count = 0;
  std::unordered_map<std::string, std::vector<std::string>> record{};

  std::vector<std::thread> threads{};
  for (auto const& entry :
       std::filesystem::recursive_directory_iterator(directory)) {
    work.push_back(entry);
  }

  std::recursive_mutex wm{};
  std::recursive_mutex rm{};

  for (int i = 0; i < 16; i++) {
    std::thread t([&work, &count, &results, &wm, &rm, i]() {
      std::string name = "thread ";
      name += ('a' + i);
      std::ofstream tfile{name};
      tfile << "Starting thread..." << std::endl;
      while (!work.empty()) {
        bool obtainedLock = false;
        decltype(work)::value_type entry;
        if (wm.try_lock() && !work.empty()) {
          entry = work[work.size() - 1];
          work.pop_back();
          obtainedLock = true;
        }
        if (obtainedLock) {
          auto hashed = SHA256Hash::ofFile(entry.path()).hex();

          if (rm.try_lock()) {
            results.push_back({hashed, entry});
            print_hashed_message(entry, count);
          }
          count++;
        }
      }
    });
    threads.push_back(std::move(t));
  }

  for (int i = 0; i < 16; i++)
    threads[i].join();

  record.reserve(count);
  while (!results.empty()) {
    auto [hash, entry] = results[results.size() - 1];
    record[hash].push_back(entry.path());
    results.pop_back();
  }

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
