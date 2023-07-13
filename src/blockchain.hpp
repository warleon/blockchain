#include <fstream>
#include <iostream>
#include <unordered_map>

#include "block.hpp"
#include "node.hpp"
#include "tsqueue.hpp"

class Blockchain : public Node {
  std::fstream localChainFile;
  tsqueue<Block> blocks;

 public:
  typedef void (*netcmd)(std::shared_ptr<connection>, message::type&);
  typedef std::unordered_map<message::category, netcmd> cmdlist_t;

 private:
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
                         message::type& msg) {
    auto cit = cmdlist.find(msg.head.cat);

    if (cit != cmdlist.end()) {
      return cit->second(client, msg);
    }
  }

 private:
  static void donothing(std::shared_ptr<connection> conn, message::type& msg) {
    std::cout << message::str[msg.head.cat] << std::endl;
  }
};
