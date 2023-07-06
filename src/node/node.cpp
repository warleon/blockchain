#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <iostream>

using asio::ip::tcp;

const int port = 1234;

int main() {
  asio::error_code ec;
  asio::io_context context;
  tcp::socket socket(context);
  return 0;
}