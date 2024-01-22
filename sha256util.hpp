#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <openssl/sha.h> 
#include <sstream>
#include <iomanip>
#include <fstream>

struct SHA256BuildError: public std::runtime_error {
  SHA256BuildError(std::string const& s);
};

struct SHA256Hash {
  std::array<unsigned char, SHA256_DIGEST_LENGTH> data; 
  std::string hex() const;

  static SHA256Hash ofString(std::string const& s);
  static SHA256Hash ofFile(std::string const& filename);
};

struct SHA256Builder {
  
private:
  SHA256_CTX ctx; 
  bool invalid{};

public:
  SHA256Builder(); 

  template<typename HasCharData>
  void update(HasCharData const& s) {
    if (invalid)
      throw std::runtime_error("Builder was already finalized");
    auto i = SHA256_Update(&ctx, s.data(), s.size());
    if (i == 0)
      throw std::runtime_error("Could not perform SHA256 update");
  }

  SHA256Hash finish();
};

