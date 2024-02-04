#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "argparse.hpp"
#include "duplicate_actions.hpp"
#include "print_util.hpp"
#include "sha256util.hpp"
namespace tom::dupdetect {
struct hashed_directory {
  std::unordered_map<std::string, std::vector<std::string>> duplicates;
  unsigned long long count;
};

hashed_directory get_duplicate_contents(std::string const& directory) {
  static auto const NUM_THREADS = std::thread::hardware_concurrency();
  std::unordered_map<std::string, std::vector<std::string>> record{};

  std::vector<std::filesystem::directory_entry> work{};
  auto iter = std::filesystem::recursive_directory_iterator(directory);
  auto end = std::filesystem::end(iter);
  while (iter != end) {
    try {
      auto const& entry = *iter++;
      work.push_back(entry);
    } catch (std::filesystem::filesystem_error& e) {
      std::cerr << "Filesystem error: " << e.what() << std::endl;
      continue;
    }
  }

  std::recursive_mutex wm{};
  std::recursive_mutex rm{};
  // std::atomic<std::size_t> atomicWorkIndex(work.size() - 1);

  std::vector<std::thread> threads{};
  std::size_t count = 0;
  std::size_t workIndex = work.size() - 1;
  for (std::decay_t<decltype(NUM_THREADS)> i = 0; i < NUM_THREADS; i++) {
    std::thread t([&work, &count, &record, &wm, &rm, &workIndex]() {
      while (true) {
        if (workIndex <= 0) {
          break;
        }
        typename decltype(work)::value_type entry;
        {
          std::unique_lock l{wm};
          entry = work[workIndex--];
        }
        auto hashed = SHA256Hash::ofFile(entry.path()).hex();
        {
          std::unique_lock l{rm};
          record[hashed].push_back(entry.path());
        }
        tom::utils::print_hashed_message(entry, count);
        count++;
      }
    });
    threads.push_back(std::move(t));
  }

  for (auto& thread : threads)
    thread.join();

  return {record, count};
}

void process_duplicates(hashed_directory& directory, auto operation) {
  for (auto& [hash, filenames] : directory.duplicates) {
    if (filenames.size() > 1) {
      operation(hash, filenames);
    }
  }
}
}  // namespace tom::dupdetect

int main(int argc, char* const argv[]) {
  tom::dupdetect::arguments args = tom::dupdetect::parse_args(argc, argv);
  auto comp = get_comparator(args.method);

  auto hdir = tom::dupdetect::get_duplicate_contents(*args.directory);
  auto [record, count] = hdir;

  std::stringstream s;
  s << "Scanned " << count - 1 << " files";
  std::string msg = s.str();
  std::cout << tom::utils::times("\x08", count)
            << tom::utils::times(" ", count - 1) << msg << std::endl;

  if (args.doRemove) {
    process_duplicates(hdir, tom::dupdetect::duplicate_remover{});
  } else {
    process_duplicates(
        hdir, tom::dupdetect::duplicate_printer{comp != nullptr, comp});
  }

  return 0;
}
