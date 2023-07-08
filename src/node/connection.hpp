#pragma once
#include <boost/asio.hpp>
#include <memory>

#include "message.hpp"
#include "tsqueue.hpp"
namespace asio = boost::asio;

class connection : public std::enable_shared_from_this<connection> {
  asio::io_context& context;
  asio::ip::tcp::socket socket;
  tsqueue<message::type>& tsqin;
  uint32_t id = 0;
  void writeHeader() {}
  void writeBody() {}
  void ReadHeader() {}
  void ReadBody() {}

 public:
  connection(asio::io_context& asioContext, asio::ip::tcp::socket socket,
             tsqueue<message::type>& qIn)
      : context(asioContext), socket(std::move(socket)), tsqin(qIn) {}
  virtual ~connection() {}
  void listen(uint32_t uid = 0) {
    if (socket.is_open()) {
      id = uid;
      ReadHeader();
    }
  }
  void connect(const asio::ip::tcp::resolver::results_type& endpoints) {
    asio::async_connect(
        socket, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
          if (!ec) {
            ReadHeader();
          }
        });
  }
  bool IsConnected() const { return socket.is_open(); }
  void Disconnect() {
    if (IsConnected()) asio::post(context, [this]() { socket.close(); });
  }
};
