#include "sha256util.hpp"

#include <openssl/sha.h>

#include <fstream>
#include <iomanip>
#include <sstream>

namespace tom::dupdetect {

SHA256BuildError::SHA256BuildError(std::string const& s)
    : std::runtime_error(s) {}

SHA256Hash SHA256Hash::ofString(std::string const& s) {
  SHA256Builder builder{};
  builder.update(s);
  return builder.finish();
}

SHA256Hash SHA256Hash::ofFile(std::string const& filename) {
  SHA256Builder builder{};
  std::ifstream f{filename, std::ios_base::binary};

  std::array<char, 4096> buf{};
  while (f.good() && !f.eof()) {
    f.read(buf.data(), 4096);
    builder.update(buf);
  }
  return builder.finish();
}

std::string SHA256Hash::hex() const {
  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    ss << std::setw(2) << std::setfill('0') << (int)data[i];
  return ss.str();
}

SHA256Builder::SHA256Builder() {
  int i = SHA256_Init(&ctx);
  if (i == 0)
    throw SHA256BuildError("Could not perform SHA256 initialization");
}

SHA256Hash SHA256Builder::finish() {
  if (invalid)
    throw std::runtime_error("Builder was already finalized");
  std::array<unsigned char, SHA256_DIGEST_LENGTH> a{};
  auto i = SHA256_Final(a.data(), &ctx);
  if (i == 0)
    throw SHA256BuildError("Could not perform SHA256 finalization");
  invalid = true;
  SHA256Hash h{};
  h.data = a;
  return h;
}

}  // namespace tom::dupdetect
