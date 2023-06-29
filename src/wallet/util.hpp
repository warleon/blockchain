#pragma once
#include <iomanip>
#include <iostream>
#include <sstream>

#include "./hash.hpp"

namespace util {
void printHash(const Hash::type& h) {
  const int* casted = (const int*)h;

  for (int i = 0; i < Hash::hashSize / sizeof(int); i++) {
    std::cout << std::hex << std::setfill('0') << std::setw(8) << casted[i]
              << " ";
  }
  std::cout << std::endl;
}

std::string asHex(const std::string& text) {
  std::stringstream ss;

  for (int i = 0; i < text.size(); i++) {
    ss << std::hex << std::setfill('0') << std::setw(2) << (int)text[i];
  }
  return ss.str();
}

}  // namespace util