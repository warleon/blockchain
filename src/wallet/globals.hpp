#pragma once
#include <array>
#include <cstring>
#include <vector>

#include "./filehash.hpp"
#include "./keygen.hpp"

namespace Interpreter {

Hash::type lastHash = {};
bool listen = true;

}  // namespace Interpreter