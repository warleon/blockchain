#pragma once
#include <boost/asio.hpp>
#include <memory>

#include "message.hpp"
#include "tsqueue.hpp"
namespace asio = boost::asio;

class connection : public std::enable_shared_from_this<connection> {
  typedef tsqueue<message::type> tsq_t;
  asio::io_context& context;
  asio::ip::tcp::socket socket;
  tsq_t& tsqin;
  tsq_t tsqout;
  message::type tempmsgin;
  uint32_t id = 0;
  void writeHeader();
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
        asio::buffer(tsqout.front().body.data(), tsqout.front().header.size),
        callback);
  }
  void writeHeader() {
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        if (tsqout.front().header.size > 0) {
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
        socket, asio::buffer(&tsqout.front().header, sizeof(message::header)),
        callback);
  }
  void readHeader();
  void addTemp() {
    tsqin.push_back(tempmsgin);
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
        if (tempmsgin.header.size > 0) {
          tempmsgin.body.resize(tempmsgin.header.size);
          readBody();
        } else {
          addTemp();
        }
      } else {
        socket.close();
      }
    };
    asio::async_read(socket,
                     asio::buffer(&tempmsgin.header, sizeof(message::header)),
                     callback);
  }

 public:
  connection(asio::io_context& asioContext, asio::ip::tcp::socket socket,
             tsqueue<message::type>& qIn)
      : context(asioContext), socket(std::move(socket)), tsqin(qIn) {}
  virtual ~connection() {}
  void listen(uint32_t uid = 0) {
    if (socket.is_open()) {
      id = uid;
      readHeader();
    }
  }
  void connect(const asio::ip::tcp::resolver::results_type& endpoints) {
    asio::async_connect(
        socket, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
          if (!ec) {
            readHeader();
          }
        });
  }
  bool IsConnected() const { return socket.is_open(); }
  void Disconnect() {
    if (IsConnected()) asio::post(context, [this]() { socket.close(); });
  }
  void writeMsg(const message::type& msg) {
    asio::post(context, [this, msg]() {
      bool bWritingMessage = !tsqout.empty();
      tsqout.push_back(msg);
      if (!bWritingMessage) {
        writeHeader();
      }
    });
  }
};
