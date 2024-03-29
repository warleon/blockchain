#pragma once
#include <boost/asio.hpp>
#include <memory>

#include "message.hpp"
#include "tsqueue.hpp"
namespace asio = boost::asio;

class connection : public std::enable_shared_from_this<connection> {
 public:
  typedef tsqueue<message::type> tsq_t;
  typedef tsqueue<message::owned_t> tsqo_t;

 private:
  asio::io_context& context;
  asio::ip::tcp::socket socket;
  tsqo_t& tsqin;
  tsq_t tsqout;
  message::type tempmsgin;
  uint32_t id = 0;

  void writeBody() {
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        tsqout.pop_front();
        if (!tsqout.empty()) {
          writeHeader();
        }
      } else {
        socket.close();
      }
    };

    asio::async_write(
        socket,
        asio::buffer(tsqout.front().body.data(), tsqout.front().head.size),
        callback);
  }
  void writeHeader() {
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        if (tsqout.front().head.size > 0) {
          writeBody();
        } else {
          tsqout.pop_front();

          if (!tsqout.empty()) {
            writeHeader();
          }
        }
      } else {
        socket.close();
      }
    };
    asio::async_write(
        socket, asio::buffer(&tsqout.front().head, sizeof(message::header_t)),
        callback);
  }
  void addTemp() {
    tsqin.push_back({this->shared_from_this(), tempmsgin});
    readHeader();
  }
  void readBody() {
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        addTemp();
      } else {
        socket.close();
      }
    };
    asio::async_read(socket,
                     asio::buffer(&tempmsgin.body[0], tempmsgin.body.size()),
                     callback);
  }
  void readHeader() {
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        if (tempmsgin.head.size > 0) {
          tempmsgin.body.resize(tempmsgin.head.size);
          readBody();
        } else {
          addTemp();
        }
      } else {
        socket.close();
      }
    };
    asio::async_read(socket,
                     asio::buffer(&tempmsgin.head, sizeof(message::header_t)),
                     callback);
  }

 public:
  connection(asio::io_context& asioContext, asio::ip::tcp::socket socket,
             tsqo_t& qIn)
      : context(asioContext), socket(std::move(socket)), tsqin(qIn) {}
  virtual ~connection() {}
  void listen(uint32_t uid = 0) {
    if (socket.is_open()) {
      id = uid;
      readHeader();
    }
  }
  void connect(const asio::ip::tcp::resolver::results_type& endpoints,
               uint32_t uid = 0) {
    id = uid;
    asio::async_connect(
        socket, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
          if (!ec) {
            readHeader();
          }
        });
  }
  bool isConnected() const { return socket.is_open(); }
  void disconnect() {
    if (isConnected()) asio::post(context, [this]() { socket.close(); });
  }
  void send(const message::type& msg) {
    asio::post(context, [this, msg]() {
      bool bWritingMessage = !tsqout.empty();
      tsqout.push_back(msg);
      if (!bWritingMessage) {
        writeHeader();
      }
    });
  }
  uint32_t getId() { return id; }
};
