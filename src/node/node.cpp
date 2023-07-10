#include "node.hpp"

#include <iostream>
#include <unordered_map>

const int port = 1234;

class Blockchain : public Node {
 public:
  typedef void (*netcmd)(message::type&);
  typedef std::unordered_map<message::category, netcmd> cmdlist_t;

 private:
  static void donothing(message::type&) {}

  const cmdlist_t cmdlist{
      {message::set_rol_as_worker, donothing},
      {message::set_rol_as_client, donothing},
      {message::check_and_add_transaction, donothing},
      {message::ask_for_block, donothing},
      {message::block_to_be_mine, donothing},
      {message::hash_of_mined_block, donothing},
  };

 public:
  Blockchain() : Node() {}

 protected:
  virtual bool OnClientConnect(std::shared_ptr<connection> client) {
    return true;
  }
  virtual void OnClientDisconnect(std::shared_ptr<connection> client) {}
  virtual void OnMessage(std::shared_ptr<connection> client,
                         message::type& msg) {}
};

int main() {
  Blockchain bc;
  bc.listen(port);
  return 0;
}