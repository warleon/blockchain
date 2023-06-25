#include "./CLI.hpp"

int main(int argc, char* argv[]) {
  bool listening = true;
  std::string line;
  while (listening) {
    std::cout << "Enter a command: ";
    std::getline(std::cin, line);
    svec splited = split(line, " \n");
    // std::cout << splited << std::endl;
    errorCode e = interpretCommand(splited, listening);
    if (e != good) std::cout << "error ocurred" << std::endl;
  }

  return 0;
}