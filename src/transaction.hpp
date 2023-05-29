#pragma once
#include <openssl/rsa.h>
#include <openssl/sha.h>

typedef unsigned char hash_t[SHA256_DIGEST_LENGTH];
typedef hash_t sign_t;
typedef RSA* public_key_t;

// typedef struct {
//   hash_t transaction_hash;        // a hash that identifies this transaction
//   sign_t key_hash_signature;      // a signature of the hash and public key
//   made
//                                   // with the private key of the owner
//   public_key_t owner_public_key;  // the public key of the owner
//   hash_t data_hash;  // a hash that identifies the data to be claimed
//   // the claimed data will not be stored in the blockchain itself
//   // but store elsewhere
// } ownership_claim;
// // size_t dummyvaltochecksizeinthelinter = sizeof(ownership_claim);
// typedef struct {
//   hash_t transaction_hash;    // a hash that identifies this transaction
//   sign_t key_hash_signature;  // a singnature from the previous owner that
//                               // certifies the transferece of ownership
//   public_key_t current_owner_public_key;
//   hash_t previous_transaction_hash;  // the hash that identifies the
//   transaction
//                                      // on wich this transaction is based
// } ownership_transfer;
// // size_t dummyvaltochecksizeinthelinter = sizeof(ownership_transfer);

// typedef union {
//   ownership_claim claim;
//   ownership_transfer transfer;
// } transaction_union;
// // size_t dummyvaltochecksizeinthelinter = sizeof(transaction_union);

typedef struct {
  hash_t self_hash;
  sign_t kh_signature;
  public_key_t owner;
  hash_t hash;
} transaction_structure;
enum transaction_enum { null, claim, transference };
// size_t dummyvaltochecksizeinthelinter = sizeof(transaction_enum);
typedef struct {
  transaction_enum type;
  transaction_structure transaction;
} transaction_t;
// size_t dummyvaltochecksizeinthelinter = sizeof(transaction_t);

