#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "block.hpp"
#include "database.hpp"
#include "node.hpp"
#include "tsqueue.hpp"

class Blockchain : public Node {
  std::string localChainName;
  std::fstream localChainFile;
  std::unique_ptr<Database> db;
  std::deque<std::shared_ptr<Block>> blocks;
  std::mutex bMutex;
  std::thread miner;
  std::unique_ptr<Block> tempBlock = std::make_unique<Block>();
  std::mutex muBlock;
  int currPOW;
  Hash::type lastAddedBlockHash{};

 public:
  Blockchain(int pow = 5, std::string fn = "localChain.bc")
      : Node(), currPOW(pow), localChainName(fn) {
    localChainFile.open(
        fn, std::ios::binary | std::ios::out | std::ios::in | std::ios::app);
    if (!localChainFile)
      localChainFile.open(fn, std::ios_base::binary | std::ios::out |
                                  std::ios::in | std::ios::trunc);
    db = std::make_unique<Database>(localChainFile);
  }

 protected:
  virtual void OnStartListening() {
    miner = std::thread([this]() {
      while (isListening()) {
        std::scoped_lock lock(bMutex);
        if (blocks.empty()) continue;
        std::cout << message::str[message::mining_block] << std::endl;
        broadcast(message::make(message::mining_block, ""), client);
        auto cb = blocks.front();
        blocks.pop_front();
        std::stringstream ss;
        auto defid = cb->getDefaultId();
        ss.write(defid.c_str(), defid.size());
        while (!cb->try_mine())
          ;
        auto nnonce = cb->getNonce();
        ss.write((char*)&nnonce, sizeof(nnonce));
        broadcast(message::make(message::hash_of_mined_block, ss.str()));
        std::stringstream ss2;
        cb->write(ss2);
        std::string blockData = ss2.str();
        db->append(blockData.c_str(), blockData.size());
        std::cout << message::str[message::block_added_to_the_chain]
                  << std::endl;
        broadcast(message::make(message::block_added_to_the_chain, ""), client);
      }
      std::terminate();
    });
  }
  virtual bool OnClientConnect(std::shared_ptr<connection> client) {
    std::cout << "recieved new connection" << std::endl;
    client->send(message::make(message::connection_success, ""));
    return true;
  }
  virtual void OnClientDisconnect(std::shared_ptr<connection> client) {
    std::cout << "a connection have been closed" << std::endl;
  }
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
        return addBlockToMine(client, msg);
      case message::hash_of_mined_block:
        return writeBlockToLocalFile(client, msg);
      case message::end_connection:
        return endConnection(client, msg);

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
    std::stringstream ss(msg.body);
    Transaction::type tran;
    Transaction::read(ss, tran);
    if (!Transaction::verify(tran))
      return conn->send(message::make(message::transaction_rejected, ""));

    conn->send(message::make(message::transaction_accepted, ""));
    std::scoped_lock sl(muBlock);
    // std::cout << tempBlock << std::endl;
    // if (tempBlock->add_transaction(tran)) return;
    bool added = tempBlock->add_transaction(tran);
    if (tempBlock->full()) {
      // share block
      std::stringstream ss2;
      tempBlock->write(ss2);
      this->broadcast(message::make(message::block_to_be_mine, ss2.str()));
      // add block to the mine queue
      std::scoped_lock lock(bMutex);
      blocks.push_back(std::move(tempBlock));
      // reset block
      tempBlock = std::make_unique<Block>();
      tempBlock->update_pow_goal(currPOW);
      tempBlock->update_previous_hash(lastAddedBlockHash);
      std::cout << message::str[message::block_to_be_mine] << std::endl;
    }
    if (!added) tempBlock->add_transaction(tran);
  }
  void setRolWorker(std::shared_ptr<connection> conn, message::type& msg) {
    bool succes = this->moveConn(conn->getId(), worker);
    if (succes) {
      conn->send(message::make(message::rol_changed_to_worker, ""));
      return;
    }
    conn->send(message::make(message::failed_to_change_rol, ""));
    return;
  }
  void setRolClient(std::shared_ptr<connection> conn, message::type& msg) {
    bool succes = this->moveConn(conn->getId(), client);
    if (succes) {
      conn->send(message::make(message::rol_changed_to_client, ""));
      return;
    }
    conn->send(message::make(message::failed_to_change_rol, ""));
    return;
  }
  void addBlockToMine(std::shared_ptr<connection> conn, message::type& msg) {
    std::stringstream ss(msg.body);
    auto tblock = std::make_shared<Block>();
    tblock->read(ss);
    // should verify but since every transaction is verified before being added
    // the process becomes redundand
    //  const auto& transactions = tblock.getTransactions();
    //  for (const auto& tran : transactions) {
    //    if (!Transaction::verify(tran)) {
    //      return;
    //    }
    //  }
    {
      std::scoped_lock lock(bMutex);
      blocks.push_back(tblock);
    }
    broadcast(message::make(message::block_accepted, ""), client);
  }
  void endConnection(std::shared_ptr<connection> conn, message::type& msg) {
    conn->disconnect();
  }
  void writeBlockToLocalFile(std::shared_ptr<connection> conn,
                             message::type& msg) {
    std::stringstream ss(msg.body);
    std::string defaultId(Hash::hashSize, '\0');
    nonce_t nnonce = 0;
    ss.read(defaultId.data(), defaultId.size());
    ss.read((char*)&nnonce, sizeof(nonce_t));

    std::scoped_lock lock(bMutex);
    auto it = std::remove_if(blocks.begin(), blocks.end(),
                             [defaultId](std::shared_ptr<Block> bptr) {
                               std::string currDefId = bptr->getDefaultId();
                               return currDefId == defaultId;
                             });
    auto dist = std::distance(it, blocks.end());
    if (dist != 1) {
      conn->send(message::make(message::block_not_found, ""));
      return;
    }
    (*it)->setNonce(nnonce);
    if (!(*it)->try_mine()) {
      conn->send(message::make(message::nonce_is_incorrect, ""));
      return;
    }
    std::stringstream sblock;
    (*it)->write(sblock);
    std::string buff = sblock.str();
    db->append(buff.c_str(), buff.size());
    broadcast(message::make(message::block_added_to_the_chain, ""), client);
  }
};
