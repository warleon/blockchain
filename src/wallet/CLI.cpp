#include "./CLI.hpp"

int main(int argc, char* argv[]) {
  std::string line;
  while (Interpreter::listen) {
    std::cout << "Enter a command: ";
    std::getline(std::cin, line);
    svec splited = split(line, " ");
    // std::cout << splited << std::endl;
    Interpreter::errorCode e = Interpreter::exec(splited);
    if (e != Interpreter::good) std::cout << "error ocurred" << std::endl;
  }

  return 0;
}