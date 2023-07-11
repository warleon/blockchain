#pragma once
#include <array>
#include <cstring>
#include <vector>

#include "./hash.hpp"
#include "./keygen.hpp"
#include "./transaction.hpp"
#include "blockchain.hpp"
namespace Interpreter {

Hash::type lastHash = {};
bool listen = true;

std::string lastSerializedPublicKey;
std::string lastSerializedPrivateKey;

Transaction::type lastTransaction;

Blockchain bcnode;
std::thread listeningThread;

}  // namespace Interpreter