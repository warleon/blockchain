#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <iostream>
#include <stdexcept>

class Key {
  RSA *rsa = nullptr;
  BIGNUM *bne = nullptr;
  BIO *bpPublic = nullptr, *bpPrivate = nullptr;

 public:
  Key(int keyLength) {
    bne = BN_new();
    if (BN_set_word(bne, RSA_F4) != 1)
      throw std::runtime_error("cant create big num");

    rsa = RSA_new();
    if (RSA_generate_key_ex(rsa, keyLength, bne, NULL) != 1)
      throw std::runtime_error("cant create  rsa");
  }
  ~Key() {
    if (rsa) RSA_free(rsa);
    if (bne) BN_free(bne);
    if (bpPublic) BIO_free_all(bpPublic);
    if (bpPrivate) BIO_free_all(bpPrivate);
  }
  void toFiles(const std::string &publicKeyFile,
               const std::string &privateKeyFile) {
    bpPublic = BIO_new_file(publicKeyFile.c_str(), "w+");
    if (PEM_write_bio_RSAPublicKey(bpPublic, rsa) != 1)
      throw std::runtime_error("cant write public key");

    bpPrivate = BIO_new_file(privateKeyFile.c_str(), "w+");
    if (PEM_write_bio_RSAPrivateKey(bpPrivate, rsa, NULL, NULL, 0, NULL,
                                    NULL) != 1)
      throw std::runtime_error("cant write private key");
  }
};

int main() {
  Key key(2048);
  key.toFiles("public_key.pem", "private_key.pem");

  return 0;
}
