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
  typedef connection::owner_t rol_t;

 private:
  tsq_t tsqin;
  std::deque<std::shared_ptr<connection>> connections;

  asio::io_context context;
  std::thread threadContext;

  std::unique_ptr<listener_t> acceptor = nullptr;
  uint32_t idCounter = 10000;
  rol_t rol = connection::client;

  bool OnClientConnect(std::shared_ptr<connection> client) { return true; }
  void OnClientDisconnect(std::shared_ptr<connection> client) {}
  void OnMessage(std::shared_ptr<connection> client, message::type& msg) {}

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
  Node() : rol(connection::client) {}
  ~Node() { stop(); }
  void broadcast(const message::type& msg, rol_t to = connection::worker) {
    bool bInvalidClientExists = false;
    for (auto& conn : connections) {
      if (conn && conn->IsConnected() && conn->owner() == to) {
        conn->send(msg);
      } else {
        OnClientDisconnect(conn);
        conn.reset();

        bInvalidClientExists = true;
      }
    }
    if (bInvalidClientExists)
      connections.erase(
          std::remove(connections.begin(), connections.end(), nullptr),
          connections.end());
  }
  void stop() {
    context.stop();
    if (threadContext.joinable()) threadContext.join();
  }
  bool listen(uint16_t port) {
    rol = connection::worker;
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
  bool connect(const std::string& host, const uint16_t port) {
    try {
      asio::ip::tcp::resolver resolver(context);
      asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(host, std::to_string(port));

      auto conn = std::make_shared<connection>(
          context, asio::ip::tcp::socket(context), tsqin);

      conn->connect(endpoints);
      conn->owner(rol);

      connections.push_back(std::move(conn));
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
};