#pragma once
#include <openssl/sha.h>

#include <fstream>

namespace Hash {

const int hashSize = SHA256_DIGEST_LENGTH;
typedef unsigned char type[hashSize];

enum errorCode {
  good = 0,
  noFile,
};

errorCode file(const std::string& filename, type& hash) {
  SHA256_CTX sha256Context;
  std::ifstream file(filename, std::ifstream::binary);
  if (!file) return noFile;
  SHA256_Init(&sha256Context);
  char buffer[8192];  // 8kb buffer
  while (file.read(buffer, sizeof(buffer))) {
    SHA256_Update(&sha256Context, buffer, file.gcount());
  }
  SHA256_Final(hash, &sha256Context);
  return good;
}

}  // namespace Hash
