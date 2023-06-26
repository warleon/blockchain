#pragma once
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "./errorCode.hpp"
#include "./filehash.hpp"
#include "./globals.hpp"
#include "./keygen.hpp"

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

namespace util {
void printHash(const Hash::type& h) {
  const int* casted = (const int*)h;

  for (int i = 0; i < Hash::hashSize / sizeof(int); i++) {
    std::cout << std::hex << std::setfill('0') << std::setw(8) << casted[i]
              << " ";
  }
  std::cout << std::endl;
}
}  // namespace util

errorCode verifySize(size_t size, size_t expectedSize) {
  if (size < expectedSize) return incompleteArguments;
  if (size > expectedSize) return execiveArguments;
  return good;
}

errorCode exec(const svec& command_args) {
  if (!command_args.size()) return noCommand;
  errorCode err = good;
  const std::string& command = command_args[0];
  const size_t argc = command_args.size() - 1;
  const svec& argv = command_args;

  if (command == "GEN_KEY_PAIR") {
    err = verifySize(argc, 2);
    if (err != good) return err;
    return Keygen::genKeyPair(argv[1], stol(argv[2]));

  } else if (command == "SHA256_FILE") {
    err = verifySize(argc, 1);
    if (err != good) return err;
    Hash::file(argv[1], lastHash);
    util::printHash(lastHash);
  } else if (command == "LAST_HASH") {
    err = verifySize(argc, 0);
    if (err != good) return err;
    util::printHash(lastHash);
  } else if (command == "EXIT") {
    err = verifySize(argc, 0);
    if (err != good) return err;
    listen = false;
  } else {
    err = noMatch;
  }
  return err;
}
}  // namespace Interpreter