#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include <cstring>
#include <iostream>
#include <string>

int main() {
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();

  // Load private key
  const char *private_key_file = "private_key.pem";
  FILE *private_key_file_ptr = fopen(private_key_file, "rb");
  RSA *rsa_private_key =
      PEM_read_RSAPrivateKey(private_key_file_ptr, NULL, NULL, NULL);
  fclose(private_key_file_ptr);

  // Load public key
  const char *public_key_file = "public_key.pem";
  FILE *public_key_file_ptr = fopen(public_key_file, "rb");
  RSA *rsa_public_key =
      PEM_read_RSAPublicKey(public_key_file_ptr, NULL, NULL, NULL);
  fclose(public_key_file_ptr);

  // Data to be signed
  unsigned char data[] = "Data to be signed";
  unsigned int data_len = strlen((char *)data);

  // Create a hash of the data
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256(data, data_len, hash);

  // Sign the hash using the private key
  unsigned char signature[RSA_size(rsa_private_key)];
  unsigned int signature_len = 0;

  int result = RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, signature,
                        &signature_len, rsa_private_key);
  if (result != 1) {
    // Error occurred while signing
    // Handle the error here
  }
  std::cout << std::hex
            << std::string(signature, signature + signature_len).c_str()
            << std::endl;

  // Verify the signature using the public key
  result = RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, signature,
                      signature_len, rsa_public_key);
  if (result != 1) {
    // Signature is not valid
    // Handle the invalid signature here
  } else {
    std::cout << " Signature is valid" << std::endl;
    // Proceed with the verified data
  }

  RSA_free(rsa_private_key);
  RSA_free(rsa_public_key);

  return 0;
}
