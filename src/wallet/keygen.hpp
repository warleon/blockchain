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
  RSA* rsa = nullptr;
  BIGNUM* bne = nullptr;
  BIO *bpPublic = nullptr, *bpPrivate = nullptr;

 public:
  Key() {}

  RSA* getKeys() { return rsa; }

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
    rsa = nullptr;
    if (bne) BN_free(bne);
    bne = nullptr;
    if (bpPublic) BIO_free_all(bpPublic);
    bpPublic = nullptr;
    if (bpPrivate) BIO_free_all(bpPrivate);
    bpPrivate = nullptr;
  }

  ~Key() { freeKey(); }
  errorCode toFiles(const std::string& publicKeyFile,
                    const std::string& privateKeyFile) {
    bpPublic = BIO_new_file(publicKeyFile.c_str(), "w+");
    if (PEM_write_bio_RSAPublicKey(bpPublic, rsa) != 1)
      return cantWritePublicKey;

    bpPrivate = BIO_new_file(privateKeyFile.c_str(), "w+");
    if (PEM_write_bio_RSAPrivateKey(bpPrivate, rsa, NULL, NULL, 0, NULL,
                                    NULL) != 1)
      return cantWritePrivateKey;

    return good;
  }

  errorCode serializePublicKey(std::string& serializedKey) {
    BIO* bio = BIO_new(BIO_s_mem());
    if (PEM_write_bio_RSAPublicKey(bio, rsa) != 1) {
      BIO_free(bio);
      return cantWritePublicKey;
    }
    char* buffer = nullptr;
    long length = BIO_get_mem_data(bio, &buffer);
    serializedKey.assign(buffer, buffer + length);
    BIO_free(bio);
    return good;
  }
  errorCode serializePrivateKey(std::string& serializedKey) {
    BIO* bio = BIO_new(BIO_s_mem());
    if (PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL) != 1) {
      BIO_free(bio);
      return cantWritePublicKey;
    }
    char* buffer = nullptr;
    long length = BIO_get_mem_data(bio, &buffer);
    serializedKey.assign(buffer, buffer + length);
    BIO_free(bio);
    return good;
  }
  errorCode readSerializedPublicKey(std::string& serializedKey) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO_write(bio, serializedKey.data(), (int)serializedKey.size());
    PEM_read_bio_RSAPublicKey(bio, &rsa, nullptr, nullptr);
    BIO_free(bio);
    return good;
  }
  errorCode readSerializedPrivateKey(std::string& serializedKey) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO_write(bio, serializedKey.data(), (int)serializedKey.size());
    PEM_read_bio_RSAPrivateKey(bio, &rsa, nullptr, nullptr);
    BIO_free(bio);
    return good;
  }
};

}  // namespace Keygen