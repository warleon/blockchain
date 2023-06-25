#pragma once
#include <array>
#include <cstring>
#include <vector>

#include "./filehash.hpp"

namespace Interpreter {

// typedef std::vector<Hash::type> hvec;

// hvec storedFileHashes(1);
Hash::type lastHash = {};
bool listen = true;

// void appendHash(const Hash::type& hash) {
//   static int count = 0;
//   if (storedFileHashes.size() == count) {
//     storedFileHashes.resize(count * 2);
//   }
//   memcpy(&storedFileHashes.data()[count], hash, Hash::hashSize);
//   count++;
// }

}  // namespace Interpreter