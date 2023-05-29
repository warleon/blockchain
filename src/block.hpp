#pragma once

#include "common_types.h"

typedef struct {
  hash_t transaction_hash;        // a hash that identifies this transaction
  sign_t key_hash_signature;      // a signature of the hash and public key made
                                  // with the private key of the owner
  public_key_t owner_public_key;  // the public key of the owner
  hash_t data_hash;  // a hash that identifies the data to be claimed
  // |v| arbitrary data to be stored in the block
  size_t data_size;
  buffer_t data;
} ownership_claim;
typedef struct {
  hash_t transaction_hash;  // a hash that identifies this transaction
  sign_t key_hash_signature;
  public_key_t current_owner_public_key;
  hash_t previus_transaction_hash;  // the hash that identifies the transaction
                                    // on wich this transaction is based
} ownership_transfer;

typedef union {
  ownership_claim claim;
  ownership_transfer transfer;
} transaction;
