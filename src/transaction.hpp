#pragma once
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