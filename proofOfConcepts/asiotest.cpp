// TCP echo server implemented with Boost.ASIO and C++11 lambdas
//
// Copyright (c) 2021 Andrzej Krzemieński (akrzemi1 at gmail dot com)
//
// Based heavily on the example by Christopher M. Kohlhoff at:
// https://www.boost.org/doc/libs/1_78_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

namespace asio = boost::asio;
namespace sys = boost::system;
using boost::asio::ip::tcp;

struct session_state {
  tcp::socket _socket;
  char _data[1024];

  explicit session_state(tcp::socket s) : _socket(std::move(s)) {}

  static void do_read(std::shared_ptr<session_state> state) {
    state->_socket.async_read_some(
        asio::buffer(state->_data),
        [state](sys::error_code ec, std::size_t length) {
          if (!ec)
            do_write(state, length);
          else if (ec == asio::error::eof)
            std::cerr << "Session done \n";
          else
            report_error("Session", ec);
        });
  }

  static void do_write(std::shared_ptr<session_state> state,
                       std::size_t length) {
    asio::async_write(state->_socket, asio::buffer(state->_data, length),
                      [state](sys::error_code ec, std::size_t /*length*/) {
                        if (!ec)
                          do_read(state);
                        else
                          report_error("Session", ec);
                      });
  }
};
void report_error(std::string_view component, sys::error_code ec) {
  std::cerr << component << " failure: " << ec << " ()" << ec.message()
            << ")\n";
}

void session(tcp::socket socket) {
  auto s = std::make_shared<session_state>(std::move(socket));
  session_state::do_read(s);
}

class listener {
  tcp::acceptor _acceptor;

 public:
  listener(asio::io_context& context, unsigned short port)
      : _acceptor{context, {tcp::v4(), port}} {}

  void operator()() { async_accept_one(); }

 private:
  void async_accept_one() {
    _acceptor.async_accept([this](sys::error_code ec, tcp::socket socket) {
      if (!ec)
        session(std::move(socket));
      else
        report_error("Listener", ec);

      async_accept_one();
    });
  }
};

int main() {
  try {
    asio::io_context context;

    asio::signal_set signals(context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { context.stop(); });

    auto listen = listener(context, 55555);
    listen();

    context.run();
    std::cerr << "Server done \n";
  } catch (std::exception& e) {
    std::cerr << "Server failure: " << e.what() << "\n";
  }
}