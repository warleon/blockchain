#include <iostream>
#include <string>
#include <utility>
#include <vector>

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

enum errorCode {
  good = 0,
  noMatch,
  noCommand,
  incompleteArguments,
  execiveArguments,
  invalidArguments
};
errorCode verifyCommandSize(size_t size, size_t expectedSize) {
  if (size < expectedSize) return incompleteArguments;
  if (size > expectedSize) return execiveArguments;
  return good;
}

errorCode interpretCommand(const svec& command_args, bool& listen) {
  if (!command_args.size()) return noCommand;
  errorCode err = good;
  const std::string& command = command_args[0];
  const size_t argc = command_args.size() - 1;
  if (command == "GEN_KEY_PAIR") {
    err = verifyCommandSize(argc, 1);
    // generate key pair
  } else if (command == "SHA256_FILE") {
    err = verifyCommandSize(argc, 1);
    // hash file
  } else if (command == "EXIT") {
    err = verifyCommandSize(argc, 0);
    listen = false;
  } else {
    err = noMatch;
  }
  return err;
}