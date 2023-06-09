#pragma once
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include <fstream>

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

void write(std::ostream& os, const transaction_t* tran) {
  // check if the whole transaction can be written
  BIO* bio = BIO_new(BIO_s_mem());
  if (!bio) throw std::runtime_error("can't open bio buffer");

  if (!PEM_write_bio_RSAPublicKey(bio, tran->transaction.owner)) {
    BIO_free(bio);
    throw std::runtime_error("can't write public key into buffer");
  }

  // write everything but the public key
  os.write((char*)tran, sizeof(transaction_t) - sizeof(public_key_t));

  // write public key
  char* buffer;
  long length = BIO_get_mem_data(bio, &buffer);
  os.write(buffer, length);
  BIO_free(bio);
}

int streamReadFunction(void* istreamPtr, unsigned char* buffer, int size) {
  std::istream& inputStream = *reinterpret_cast<std::istream*>(istreamPtr);
  inputStream.read(reinterpret_cast<char*>(buffer), size);
  return static_cast<int>(inputStream.gcount());
}

void read(std::istream& is, const transaction_t* tran) {
  // read everything but the public key
  is.read((char*)tran, sizeof(transaction_t) - sizeof(public_key_t));

  BIO_METHOD* bioMethod = BIO_meth_new(BIO_TYPE_MEM, "Custom BIO");
  if (!bioMethod) {
    throw std::runtime_error("Failed to create BIO_METHOD");
  }
  BIO_METHOD* defaultBioMethod = BIO_meth_new(BIO_TYPE_MEM, "Custom BIO");
  // Copy necessary fields from the default method
  BIO_meth_set_write(bioMethod, BIO_meth_get_write(BIO_s_mem()));
  BIO_meth_set_ctrl(bioMethod, BIO_meth_get_ctrl(BIO_s_mem()));
  BIO_meth_set_create(bioMethod, BIO_meth_get_create(BIO_s_mem()));
  BIO_meth_set_destroy(bioMethod, BIO_meth_get_destroy(BIO_s_mem()));

  BIO* bio = BIO_new(bioMethod);
  if (!bio) {
    BIO_meth_free(bioMethod);
    throw std::runtime_error("Failed to create BIO");
  }
  BIO_set_data(bio, &inputStream);
  BIO_set_init(bio, 1);
  BIO_set_mem_eof_return(bio, 0);
  BIO_set_read(bio, streamReadFunction);
}