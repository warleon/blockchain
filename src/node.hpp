#pragma once
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <string>

#include "connection.hpp"
#include "tsqueue.hpp"

namespace asio = boost::asio;

class Node {
 public:
  typedef connection::tsqo_t tsq_t;
  typedef asio::ip::tcp::acceptor listener_t;
  enum rol_t { worker, client };

 private:
  tsq_t tsqin;
  std::deque<std::shared_ptr<connection>> clients, workers;

  asio::io_context context;
  std::thread threadContext;

  std::unique_ptr<listener_t> acceptor = nullptr;
  uint32_t idCounter = 10000;

  void waitForConection() {
    auto callback = [this](std::error_code ec, asio::ip::tcp::socket socket) {
      if (!ec) {
        std::shared_ptr<connection> newconn =
            std::make_shared<connection>(context, std::move(socket), tsqin);
        if (OnClientConnect(newconn)) {
          clients.push_back(std::move(newconn));
          clients.back()->listen(idCounter++);
        }
      }
      waitForConection();
    };
    acceptor->async_accept(callback);
  }
  bool listening = false;

 public:
  Node() {}
  ~Node() {
    stop();
    clients.clear();
    workers.clear();
  }
  void broadcast(const message::type& msg, rol_t to = worker) {
    bool valid = false;
    auto& conns = (to == worker) ? workers : clients;
    for (auto& conn : conns) {
      if (conn && conn->IsConnected()) {
        conn->send(msg);
      } else {
        OnClientDisconnect(conn);
        conn.reset();

        valid = true;
      }
    }

    if (valid)
      conns.erase(std::remove(conns.begin(), conns.end(), nullptr),
                  conns.end());
  }
  void stop() {
    context.stop();
    if (threadContext.joinable()) threadContext.join();
    listening = false;
  }
  bool listen(uint16_t port) {
    acceptor = std::make_unique<listener_t>(
        context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    try {
      waitForConection();

      threadContext = std::thread([this]() { context.run(); });
    } catch (std::exception& e) {
      listening = false;
    }
    listening = true;
    return listening;
  }
  bool connect(const std::string& host, const uint16_t port) {
    try {
      asio::ip::tcp::resolver resolver(context);
      asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(host, std::to_string(port));

      auto conn = std::make_shared<connection>(
          context, asio::ip::tcp::socket(context), tsqin);

      conn->connect(endpoints);

      workers.push_back(std::move(conn));
    } catch (std::exception& e) {
      return false;
    }
    return true;
  }

  void update(size_t nMaxMessages = -1, bool bWait = false) {
    if (bWait) tsqin.wait();

    size_t nMessageCount = 0;
    while (nMessageCount < nMaxMessages && !tsqin.empty()) {
      auto msg = tsqin.pop_front();
      OnMessage(msg.remote, msg.msg);

      nMessageCount++;
    }
  }
  bool isListening() { return listening; }

 protected:
  virtual bool OnClientConnect(std::shared_ptr<connection> client) {
    return true;
  }
  virtual void OnClientDisconnect(std::shared_ptr<connection> client) {}
  virtual void OnMessage(std::shared_ptr<connection> client,
                         message::type& msg) {}
};