#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "block.hpp"
#include "node.hpp"
#include "tsqueue.hpp"

class Blockchain : public Node {
  std::fstream localChainFile;
  tsqueue<Block> blocks;
  Block tempBlock;
  std::mutex muBlock;
  int currPOW;
  Hash::type lastAddedBlockHash{};

 public:
  Blockchain(int pow = 0) : Node(), currPOW(pow) {}

 protected:
  virtual bool OnClientConnect(std::shared_ptr<connection> client) {
    return true;
  }
  virtual void OnClientDisconnect(std::shared_ptr<connection> client) {}
  virtual void OnMessage(std::shared_ptr<connection> client,
                         message::type& msg) {
    switch (msg.head.cat) {
      case message::set_rol_as_worker:
        return setRolWorker(client, msg);
      case message::set_rol_as_client:
        return setRolClient(client, msg);
      case message::check_and_add_transaction:
        return checkAddTransaction(client, msg);
      case message::ask_for_block:
        return donothing(client, msg);
      case message::block_to_be_mine:
        return donothing(client, msg);
      case message::hash_of_mined_block:
        return donothing(client, msg);

      default:
        return donothing(client, msg);
    }
  }

 private:
  void donothing(std::shared_ptr<connection> conn, message::type& msg) {
    std::cout << message::str[msg.head.cat] << std::endl;
  }
  void checkAddTransaction(std::shared_ptr<connection> conn,
                           message::type& msg) {
    std::cout << message::str[msg.head.cat] << std::endl;
    std::stringstream ss(msg.body);
    Transaction::type tran;
    Transaction::read(ss, tran);
    if (!Transaction::verify(tran))
      return conn->send(message::make(message::transaction_rejected, ""));

    conn->send(message::make(message::transaction_accepted, ""));
    std::scoped_lock sl(muBlock);
    if (tempBlock.add_transaction(tran)) return;
    blocks.push_back(tempBlock);
    tempBlock.reset();
    tempBlock.update_pow_goal(currPOW);
    tempBlock.update_previous_hash(lastAddedBlockHash);

    tempBlock.add_transaction(tran);
  }
  void setRolWorker(std::shared_ptr<connection> conn, message::type& msg) {
    uint32_t id;
    if (msg.head.size != sizeof(id)) {
      conn->send(message::make(message::wrong_message_body, ""));
      return;
    }
    std::stringstream ss(msg.body);
    ss.read((char*)&id, sizeof(id));
    bool succes = this->moveConn(id, worker);
    if (succes) {
      conn->send(message::make(message::rol_changed_to_worker, ""));
      return;
    }
    conn->send(message::make(message::failed_to_change_rol, ""));
    return;
  }
  void setRolClient(std::shared_ptr<connection> conn, message::type& msg) {
    uint32_t id;
    if (msg.head.size != sizeof(id)) {
      conn->send(message::make(message::wrong_message_body, ""));
      return;
    }
    std::stringstream ss(msg.body);
    ss.read((char*)&id, sizeof(id));
    bool succes = this->moveConn(id, client);
    if (succes) {
      conn->send(message::make(message::rol_changed_to_client, ""));
      return;
    }
    conn->send(message::make(message::failed_to_change_rol, ""));
    return;
  }
};
