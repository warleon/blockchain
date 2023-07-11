#include "./CLI.hpp"

int main(int argc, char* argv[]) {
  std::string line;
  while (Interpreter::listen) {
    std::cout << "Enter a command: ";
    std::getline(std::cin, line);
    svec splited = split(line, " ");
    errorCode e = Interpreter::exec(splited);
    if (e != good) std::cout << what.at(e) << std::endl;
  }
  if (Interpreter::bcnode.isListening()) Interpreter::bcnode.stop();
  if (Interpreter::listeningThread.joinable())
    Interpreter::listeningThread.join();

  return 0;
}