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
  typedef connection::tsq_t tsq_t;
  typedef asio::ip::tcp::acceptor listener_t;

 private:
  tsq_t tsqin;
  std::deque<std::shared_ptr<connection>> connections;

  asio::io_context context;
  std::thread threadContext;

  std::unique_ptr<listener_t> acceptor = nullptr;
  uint32_t idCounter = 10000;

  bool OnClientConnect(std::shared_ptr<connection> client) { return true; }
  void OnClientDisconnect(std::shared_ptr<connection> client) {}
  void OnMessage(std::shared_ptr<connection> client, message& msg) {}

  void waitForConection() {
    auto callback = [this](std::error_code ec, asio::ip::tcp::socket socket) {
      if (!ec) {
        std::shared_ptr<connection> newconn =
            std::make_shared<connection>(context, std::move(socket), tsqin);
        if (OnClientConnect(newconn)) {
          connections.push_back(std::move(newconn));
          connections.back()->listen(idCounter++);
        }
      }
      waitForConection();
    };
    acceptor->async_accept(callback);
  }

 public:
  bool listen(uint16_t port) {
    acceptor = std::make_unique<listener_t>(
        context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    try {
      waitForConection();

      threadContext = std::thread([this]() { context.run(); });
    } catch (std::exception& e) {
      return false;
    }
    return true;
  }
  void stop() {
    context.stop();
    if (threadContext.joinable()) threadContext.join();
  }
  Node() {}
  ~Node() { stop(); }
};