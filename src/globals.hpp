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
std::thread listeningThread = std::thread([]() {
  std::cout << "CLI started" << std::endl;
  while (Interpreter::listen) {
    Interpreter::bcnode.update();
  }
  std::cout << "CLI exiting" << std::endl;
});

}  // namespace Interpreter