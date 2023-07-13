#pragma once

#include <openssl/sha.h>

#include <array>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "hash.hpp"
#include "transaction.hpp"

typedef int nonce_t;

class Block {
  // the number of transactions is arbitrary, shall be changed
  // later to match a prefered block size
  static const size_t t_max = 1;
  Hash::type self_hash{};
  size_t pow_goal;
  Hash::type previous_hash;
  nonce_t nonce = 0;
  size_t t_count = 0;
  Transaction::type transactions[t_max]{};

 public:
  Block(const Hash::type& previous_hash_) {
    update_previous_hash(previous_hash_);
  }
  Block(const Hash::type& previous_hash_, size_t pow_goal_)
      : pow_goal(pow_goal_) {
    update_previous_hash(previous_hash_);
  }
  Block() {}  // empty constructor ment o be use when loading block from disk
  ~Block() {}
  void update_previous_hash(const Hash::type& previous_hash_) {
    for (int i = 0; i < Hash::hashSize; i++) {
      this->previous_hash[i] = previous_hash_[i];
    }
  }
  void update_pow_goal(size_t pow_goal_) { pow_goal = pow_goal_; }

  bool add_transaction(const Transaction::type& transaction) {
    if (t_count >= t_max) return false;
    transactions[t_count++] = transaction;
    return true;
  }

  // applies  one iteration of the POW mining proccess returns true if the
  // POW goal is achieved flase otherwise
  bool try_mine() {
    // sets a random nonce
    srand((unsigned)time(NULL));
    nonce = rand();
    // clears self hash
    for (int i = 0; i < Hash::hashSize; i++) this->self_hash[i] = Hash::null[i];

    std::stringstream ss;
    write(ss);
    std::string buffer = ss.str();
    SHA256((unsigned char*)buffer.c_str(), buffer.size(), self_hash);
    // check if the hash matches the pow goal
    for (int i = 0; i < pow_goal && i < Hash::hashSize; i++) {
      if (self_hash[i] != 0) return false;
    }
    return true;
  }
  void write(std::ostream& os) {
    os.write((char*)self_hash, sizeof(Hash::type));
    os.write((char*)previous_hash, sizeof(Hash::type));
    os.write((char*)&pow_goal, sizeof(pow_goal));
    os.write((char*)&nonce, sizeof(nonce));
    for (int i = 0; i < t_max; i++) Transaction::write(os, transactions[i]);
  }
  void read(std::istream& is) {
    is.read((char*)self_hash, sizeof(Hash::type));
    is.read((char*)previous_hash, sizeof(Hash::type));
    is.read((char*)&pow_goal, sizeof(pow_goal));
    is.read((char*)&nonce, sizeof(nonce));
    for (int i = 0; i < t_max; i++) Transaction::read(is, transactions[i]);
  }
};