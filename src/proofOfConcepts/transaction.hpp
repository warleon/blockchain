#pragma once
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include <fstream>
#include <vector>

typedef unsigned char hash_t[SHA256_DIGEST_LENGTH];
typedef hash_t sign_t;
typedef RSA* public_key_t;

typedef struct {
  hash_t self_hash;
  sign_t kh_signature;
  hash_t hash;
  public_key_t owner;
} transaction_structure;
enum transaction_enum { null, claim, transference };
// size_t dummyvaltochecksizeinthelinter = sizeof(transaction_enum);
typedef struct {
  transaction_enum type;
  transaction_structure transaction;
} transaction_t;
// size_t dummyvaltochecksizeinthelinter = sizeof(transaction_t);

// Function to serialize the RSA key to a byte array
std::vector<unsigned char> serializeRSAKey(public_key_t key) {
  std::vector<unsigned char> serializedKey;

  // Create a memory BIO to hold the serialized key
  BIO* bio = BIO_new(BIO_s_mem());

  // Write the RSA key to the BIO in PEM format
  if (PEM_write_bio_RSAPublicKey(bio, key) != 0) {
    // Read the data from the BIO into a buffer
    char* buffer;
    long length = BIO_get_mem_data(bio, &buffer);

    // Copy the data from the buffer to the serializedKey vector
    serializedKey.assign(buffer, buffer + length);
  }

  // Free the BIO resources
  BIO_free(bio);

  return serializedKey;
}

// Function to deserialize the RSA key from a byte array
public_key_t deserializeRSAKey(
    const std::vector<unsigned char>& serializedKey) {
  public_key_t key = nullptr;

  // Create a memory BIO to hold the serialized key data
  BIO* bio = BIO_new(BIO_s_mem());

  // Write the serialized key data to the BIO
  BIO_write(bio, serializedKey.data(), static_cast<int>(serializedKey.size()));

  // Read the RSA key from the BIO
  PEM_read_bio_RSAPublicKey(bio, &key, nullptr, nullptr);

  // Free the BIO resources
  BIO_free(bio);

  return key;
}

// Function to write the transaction_t structure to a file
void writeTransactionToFile(const transaction_t& transaction,
                            std::ofstream& outFile) {
  // Serialize the RSA key member
  std::vector<unsigned char> serializedKey =
      serializeRSAKey(transaction.transaction.owner);

  // Write the size of the serialized key to the file
  size_t keySize = serializedKey.size();
  outFile.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));

  // Write the serialized key data to the file
  outFile.write(reinterpret_cast<const char*>(serializedKey.data()), keySize);

  // Write the remaining transaction structure to the file
  outFile.write(reinterpret_cast<const char*>(&transaction),
                sizeof(transaction));

  outFile.close();
}

// Function to read the transaction_t structure from a file
void readTransactionFromFile(transaction_t& transaction,
                             std::ifstream& inFile) {
  // Read the size of the serialized key from the file
  size_t keySize;
  inFile.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));

  // Read the serialized key data from the file
  std::vector<unsigned char> serializedKey(keySize);
  inFile.read(reinterpret_cast<char*>(serializedKey.data()), keySize);

  // Deserialize the RSA key member
  transaction.transaction.owner = deserializeRSAKey(serializedKey);

  // Read the remaining transaction structure from the file
  inFile.read(reinterpret_cast<char*>(&transaction), sizeof(transaction));

  inFile.close();
}