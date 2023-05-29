#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <iostream>

void printPrivateKey(RSA* privateKey) {
  BIO* bio = BIO_new(BIO_s_mem());
  if (!bio) {
    std::cerr << "Failed to create BIO" << std::endl;
    return;
  }

  if (PEM_write_bio_RSAPublicKey(bio, privateKey) != 1) {
    std::cerr << "Failed to write private key to BIO" << std::endl;
    BIO_free(bio);
    return;
  }

  char* buffer;
  long length = BIO_get_mem_data(bio, &buffer);
  if (length > 0) {
    std::cout << "Private Key:" << std::endl;
    std::cout.write(buffer, length);
    std::cout << std::endl;
  }

  BIO_free(bio);
}

int main() {
  std::string privateKeyPath = "public_key.pem";

  FILE* privateKeyFile = fopen(privateKeyPath.c_str(), "rb");
  if (!privateKeyFile) {
    std::cerr << "Failed to open private key file: " << privateKeyPath
              << std::endl;
    return 1;
  }

  RSA* privateKey =
      PEM_read_RSAPublicKey(privateKeyFile, nullptr, nullptr, nullptr);
  fclose(privateKeyFile);

  if (!privateKey) {
    std::cerr << "Failed to load private key" << std::endl;
    return 1;
  }

  printPrivateKey(privateKey);

  RSA_free(privateKey);

  return 0;
}
