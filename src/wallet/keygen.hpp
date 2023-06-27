#pragma once
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "./errorCode.hpp"
namespace fs = std::filesystem;
namespace Keygen {

class Key {
  RSA *rsa = nullptr;
  BIGNUM *bne = nullptr;

 public:
  Key() {}

  errorCode generate(int keyLength) {
    bne = BN_new();
    if (BN_set_word(bne, RSA_F4) != 1) return cantCreateBigNum;

    rsa = RSA_new();
    if (RSA_generate_key_ex(rsa, keyLength, bne, NULL) != 1)
      return cantCreateRSA;

    return good;
  }
  void freeKey() {
    if (rsa) RSA_free(rsa);
    if (bne) BN_free(bne);
  }

  ~Key() { freeKey(); }
  errorCode toFiles(const fs::path publicKeyFile,
                    const fs::path privateKeyFile) {
    FILE *private_key_file_ptr = fopen(privateKeyFile.c_str(), "w");
    FILE *public_key_file_ptr = fopen(publicKeyFile.c_str(), "w");
    if (PEM_write_RSAPublicKey(public_key_file_ptr, rsa) != 1) {
      fclose(private_key_file_ptr);
      fclose(public_key_file_ptr);
      return cantWritePublicKey;
    }

    if (PEM_write_RSAPrivateKey(private_key_file_ptr, rsa, NULL, NULL, 0, NULL,
                                NULL) != 1) {
      fclose(private_key_file_ptr);
      fclose(public_key_file_ptr);
      return cantWritePrivateKey;
    }

    return good;
  }
};

}  // namespace Keygen