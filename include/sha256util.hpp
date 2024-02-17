#ifndef SHA256UTIL_HPP
#define SHA256UTIL_HPP

#include <openssl/sha.h>

#include <array>
#include <stdexcept>
#include <string>

namespace tom::dupdetect {

struct SHA256BuildError : public std::runtime_error {
  explicit SHA256BuildError(std::string const& s);
};

struct SHA256Hash {
  std::array<unsigned char, SHA256_DIGEST_LENGTH> data;

  [[nodiscard]] std::string hex() const;

  static SHA256Hash ofString(std::string const& s);

  static SHA256Hash ofFile(std::string const& filename);
};

template<class T>
concept HasCharData = requires(T t) {
  { t.data() } -> std::same_as<char*>;
  { t.size() } -> std::same_as<std::size_t>;
};

struct SHA256Builder {
 private:
  SHA256_CTX ctx{};
  bool invalid{};

 public:
  SHA256Builder();

  void update(HasCharData auto const& s) {
    if (invalid)
      throw std::runtime_error("Builder was already finalized");
    auto i = SHA256_Update(&ctx, s.data(), s.size());
    if (i == 0)
      throw std::runtime_error("Could not perform SHA256 update");
  }

  SHA256Hash finish();
};

}  // namespace tom::dupdetect
#endif
