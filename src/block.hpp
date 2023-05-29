#pragma once

#include <openssl/sha.h>

#include <array>
#include <cstdlib>

typedef size_t nonce_t;
typedef unsigned char hash_t[SHA256_DIGEST_LENGTH];
typedef hash_t sign_t;
typedef size_t public_key_t;

typedef struct {
  hash_t transaction_hash;        // a hash that identifies this transaction
  sign_t key_hash_signature;      // a signature of the hash and public key made
                                  // with the private key of the owner
  public_key_t owner_public_key;  // the public key of the owner
  hash_t data_hash;  // a hash that identifies the data to be claimed
  // the claimed data will not be stored in the blockchain itself
  // but store elsewhere
} ownership_claim;
typedef struct {
  hash_t transaction_hash;    // a hash that identifies this transaction
  sign_t key_hash_signature;  // a singnature from the previous owner that
                              // certifies the transferece of ownership
  public_key_t current_owner_public_key;
  hash_t previous_transaction_hash;  // the hash that identifies the transaction
                                     // on wich this transaction is based
} ownership_transfer;

typedef union {
  ownership_claim claim;
  ownership_transfer transfer;
} transaction_union;
enum transaction_enum { null, claim, transference };
typedef struct {
  transaction_enum type;
  transaction_union transaction;
} transaction_t;
// size_t dummyvaltochecksizeinthelinter = sizeof(transaction_t);

class Block {
  // the number of transactions is arbitrary, shall be change
  // later to match a prefered block size
  static const size_t t_max = 8;
  hash_t self_hash{};
  size_t pow_goal;
  hash_t previous_hash;
  nonce_t nonce = 0;
  size_t t_count = 0;
  transaction_t transactions[t_max] = {};

 public:
  Block(const hash_t& previous_hash_) { update_previous_hash(previous_hash_); }
  Block(const hash_t& previous_hash_, size_t pow_goal_) : pow_goal(pow_goal_) {
    update_previous_hash(previous_hash_);
  }
  Block() {}  // empty constructor ment o be use when loading block from disk
  ~Block() {}
  void update_previous_hash(const hash_t& previous_hash_) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
      this->previous_hash[i] = previous_hash_[i];
    }
  }
  void update_pow_goal(size_t pow_goal_) { pow_goal = pow_goal_; }

  bool add_transaction(const transaction_t& transaction) {
    if (t_count >= t_max) return false;
    transactions[t_count++] = transaction;
    return true;
  }

  // applies  one iteration of the POW mining proccess returns true if the
  // POW is achieved flase otherwise
  bool try_mine() {
    // sets a random nonce
    srand((unsigned)time(NULL));
    nonce = rand();
    // applies the sha256 to the transactions+nonce+previous_hash
    SHA256(previous_hash,  // the following size is ment to be bigger than
                           // this buffer size in order to catch the rest
                           // of the class members assuming they are adjacent
                           // in memory
           (sizeof(previous_hash) + sizeof(nonce) + sizeof(t_count) +
            sizeof(transactions)),
           self_hash);
    // check if the hash matches the pow goal
    for (int i = 0; i < pow_goal && i < SHA256_DIGEST_LENGTH; i++) {
      if (self_hash[i] != 0) return false;
    }
    return true;
  }
  void write() {}
  void read() {}
};