#pragma once
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "./errorCode.hpp"
#include "./globals.hpp"
#include "./hash.hpp"
#include "./keygen.hpp"
#include "./util.hpp"

typedef std::vector<std::string> svec;

auto split(const std::string& line, const std::string& delim) {
  svec result;
  std::string word;

  for (const char c : line) {
    if (delim.find(c) != std::string::npos) {
      result.push_back(std::move(word));
    } else {
      word += c;
    }
  }
  result.push_back(std::move(word));

  return std::move(result);
}
std::ostream& operator<<(std::ostream& os, const svec& words) {
  os << "[ ";
  for (const std::string& word : words) {
    os << "\"" << word << "\", ";
  }
  os << " ]";
  return os;
}

namespace Interpreter {

typedef errorCode (*command_t)(int, const svec&);
errorCode generateKeyPair(int, const svec&);
errorCode hashFile(int, const svec&);
errorCode printLastHash(int, const svec&);
errorCode stopListening(int, const svec&);
errorCode printAvailableCommands(int, const svec&);
errorCode createTransaction(int, const svec&);
errorCode claimFile(int, const svec&);

const std::unordered_map<std::string, command_t> commandList{
    // {"GEN_KEY_PAIR", generateKeyPair},
    // {"SHA256_FILE", hashFile},
    // {"LAST_HASH", printLastHash},
    {"CLAIM", claimFile},
    {"EXIT", stopListening},
    {"HELP", printAvailableCommands},
    // {"CLAIM_HASHED_FILE", createTransaction}
};

errorCode exec(const svec& command_args) {
  if (!command_args.size()) return noCommand;
  errorCode err = good;
  const std::string& command = command_args[0];
  std::string capCom(command);
  std::transform(capCom.begin(), capCom.end(), capCom.begin(), ::toupper);
  const int argc = command_args.size();
  const svec& argv = command_args;

  auto cit = commandList.find(capCom);

  if (cit != commandList.end()) {
    return cit->second(argc, argv);
  }
  return noMatch;
}
errorCode verifySize(int size, int expectedSize) {
  if (size < expectedSize) return incompleteArguments;
  if (size > expectedSize) return execiveArguments;
  return good;
}

errorCode generateKeyPair(int argc, const svec& argv) {
  static const int keysize[] = {1024, 2048, 4096, 8192};

  errorCode err = verifySize(argc, 2);
  if (err != good) return err;

  // fs::path path = argv[1];
  int strength = stol(argv[1]);

  Keygen::Key newkey;
  newkey.generate(keysize[strength]);

  err = newkey.serializePublicKey(lastSerializedPublicKey);
  if (err != good) return err;
  err = newkey.serializePrivateKey(lastSerializedPrivateKey);
  if (err != good) return err;
  std::cout << lastSerializedPublicKey << lastSerializedPrivateKey << std::endl;
  return err;
}
errorCode hashFile(int argc, const svec& argv) {
  errorCode err = verifySize(argc, 2);
  if (err != good) return err;
  err = Hash::file(argv[1], lastHash);
  if (err != good) return err;
  util::printHash(lastHash);
  return good;
}
errorCode printLastHash(int argc, const svec& argv) {
  errorCode err = verifySize(argc, 1);
  if (err != good) return err;
  util::printHash(lastHash);
  return good;
}
errorCode stopListening(int argc, const svec& argv) {
  errorCode err = verifySize(argc, 1);
  if (err != good) return err;
  listen = false;
  return good;
}
errorCode printAvailableCommands(int argc, const svec& argv) {
  errorCode err = verifySize(argc, 1);
  if (err != good) return err;
  for (const auto& cmd : commandList) {
    std::cout << cmd.first << std::endl;
  }
  return good;
}
errorCode createTransaction(int argc, const svec& argv) {
  errorCode err = verifySize(argc, 1);
  if (err != good) return err;
  lastTransaction.type = Transaction::claim;
  memcpy(lastTransaction.hash, lastHash, Hash::hashSize);
  lastTransaction.publickey = lastSerializedPublicKey;
  Transaction::sign(lastTransaction, lastSerializedPrivateKey);
  Transaction::setId(lastTransaction);
  std::cout << lastTransaction << std::endl;
  return good;
}

errorCode claimFile(int argc, const svec& argv) {
  errorCode err = verifySize(argc, 3);
  if (err != good) return err;
  err = hashFile(2, {argv[0], argv[1]});
  if (err != good) return err;
  err = generateKeyPair(2, {argv[0], argv[2]});
  if (err != good) return err;
  err = createTransaction(1, {argv[0]});
  if (err != good) return err;
  return good;
}
}  // namespace Interpreter