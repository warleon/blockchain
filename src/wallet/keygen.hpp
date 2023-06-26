#pragma once
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "./errorCode.hpp"
namespace fs = std::filesystem;
namespace Keygen {

std::vector<errorCode> errors{};

class Key {
  RSA *rsa = nullptr;
  BIGNUM *bne = nullptr;
  BIO *bpPublic = nullptr, *bpPrivate = nullptr;

 public:
  Key(int keyLength) {
    bne = BN_new();
    if (BN_set_word(bne, RSA_F4) != 1) errors.push_back(cantCreateBigNum);

    rsa = RSA_new();
    if (RSA_generate_key_ex(rsa, keyLength, bne, NULL) != 1)
      errors.push_back(cantCreateRSA);
  }
  ~Key() {
    if (rsa) RSA_free(rsa);
    if (bne) BN_free(bne);
    if (bpPublic) BIO_free_all(bpPublic);
    if (bpPrivate) BIO_free_all(bpPrivate);
  }
  errorCode toFiles(const fs::path publicKeyFile,
                    const fs::path privateKeyFile) {
    if (errors.size()) return multipleErrors;
    bpPublic = BIO_new_file(publicKeyFile.c_str(), "w+");
    if (PEM_write_bio_RSAPublicKey(bpPublic, rsa) != 1)
      return cantWritePublicKey;

    bpPrivate = BIO_new_file(privateKeyFile.c_str(), "w+");
    if (PEM_write_bio_RSAPrivateKey(bpPrivate, rsa, NULL, NULL, 0, NULL,
                                    NULL) != 1)
      return cantWritePrivateKey;

    return good;
  }
};

errorCode genKeyPair(fs::path path, int strength) {
  static int keysize[] = {1024, 2048, 4096, 8192};
  Key pair(keysize[strength]);
  fs::create_directories(path);
  return pair.toFiles(path / "pubkey.pem", path / "privkey.pem");
}

}  // namespace Keygen