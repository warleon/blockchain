#pragma once

#include <string>

#include "./errorCode.hpp"
#include "./hash.hpp"

namespace Transaction {

enum category { null, claim, transference };
typedef struct {
  category type;
  Hash::type hash;
  std::string publickey;
  std::string signature;
  Hash::type id;
} type;

errorCode sign(type& trans, std::string privatekey) {}
errorCode setId(type&) {}

}  // namespace Transaction