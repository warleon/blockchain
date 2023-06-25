#include "./CLI.hpp"

int main(int argc, char* argv[]) {
  bool listening = true;
  std::string line;
  while (listening) {
    std::cout << "Enter a command: ";
    std::getline(std::cin, line);
    svec splited = split(line, " ");
    // std::cout << splited << std::endl;
    Interpreter::errorCode e = Interpreter::exec(splited, listening);
    if (e != Interpreter::good) std::cout << "error ocurred" << std::endl;
  }

  return 0;
}