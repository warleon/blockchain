#include "./CLI.hpp"

int main(int argc, char* argv[]) {
  std::string line;
  while (Interpreter::listen) {
    std::cout << "Enter a command: ";
    std::getline(std::cin, line);
    svec splited = split(line, " ");
    // std::cout << splited << std::endl;
    errorCode e = Interpreter::exec(splited);
    if (e != good) std::cout << what.at(e) << std::endl;
    // if (e == multipleErrors) {
    //   for (errorCode ec : Keygen::errors) {
    //     std::cout << what.at(ec) << std::endl;
    //   }
    //   Keygen::errors.clear();
    // }
  }

  return 0;
}