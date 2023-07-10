#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"

class connection : public std::enable_shared_from_this<connection> {
  typedef std::string msg_t;
  typedef olc::net::tsqueue<msg_t> tsq_t;

 public:
  // A connection is "owned" by either a server or a client, and its
  // behaviour is slightly different bewteen the two.
  enum class owner { server, client };

 public:
  // Constructor: Specify Owner, connect to context, transfer the socket
  //				Provide reference to incoming message queue
  connection(owner parent, asio::io_context& asioContext,
             asio::ip::tcp::socket socket, tsq_t& qIn)
      : m_asioContext(asioContext),
        m_socket(std::move(socket)),
        m_qMessagesIn(qIn) {
    m_nOwnerType = parent;
  }

  virtual ~connection() {}

  // This ID is used system wide - its how clients will understand other clients
  // exist across the whole system.
  uint32_t GetID() const { return id; }

 public:
  void listen(uint32_t uid = 0) {
    if (m_socket.is_open()) {
      id = uid;
      ReadHeader();
    }
  }

  void Connect(const asio::ip::tcp::resolver::results_type& endpoints) {
    // Only clients can connect to servers
    // Request asio attempts to connect to an endpoint
    asio::async_connect(
        m_socket, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
          if (!ec) {
            ReadHeader();
          }
        });
  }

  void Disconnect() {
    if (IsConnected())
      asio::post(m_asioContext, [this]() { m_socket.close(); });
  }

  bool IsConnected() const { return m_socket.is_open(); }

  // Prime the connection to wait for incoming messages
  void StartListening() {}

 public:
  // ASYNC - Send a message, connections are one-to-one so no need to specifiy
  // the target, for a client, the target is the server and vice versa
  void Send(const msg_t& msg) {
    asio::post(m_asioContext, [this, msg]() {
      // If the queue has a message in it, then we must
      // assume that it is in the process of asynchronously being written.
      // Either way add the message to the queue to be output. If no messages
      // were available to be written, then start the process of writing the
      // message at the front of the queue.
      bool bWritingMessage = !m_qMessagesOut.empty();
      m_qMessagesOut.push_back(msg);
      if (!bWritingMessage) {
        async_send();
      }
    });
  }

 private:
  // ASYNC - Prime context to write a message header
  void async_send() {
    // If this function is called, we know the outgoing message queue must have
    // at least one message to send. So allocate a transmission buffer to hold
    // the message, and issue the work - asio, send these bytes
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        m_qMessagesOut.pop_front();
        if (!m_qMessagesOut.empty()) {
          async_send();
        }
      } else {
        std::cout << "[" << id << "] WRITE FAILED.\n";
        m_socket.close();
      }
    };

    asio::async_write(m_socket,
                      asio::buffer(&m_qMessagesOut.front().data(),
                                   m_qMessagesOut.front().size()),
                      callback);
  }

  // ASYNC - Prime context ready to read a message header
  void recieve_size() {
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        if (in_msg_size > 0) {
          m_msgTemporaryIn.resize(in_msg_size);
          ReadBody();
        }
      } else {
        std::cout << "[" << id << "] Read size failed.\n";
        m_socket.close();
      }
    };
    asio::async_read(m_socket, asio::buffer(&in_msg_size, sizeof(size_t)),
                     callback);
  }

  // ASYNC - Prime context ready to read a message body
  void ReadBody() {
    auto callback = [this](std::error_code ec, std::size_t length) {
      if (!ec) {
        AddToIncomingMessageQueue();
      } else {
        // As above!
        std::cout << "[" << id << "] Read Body Fail.\n";
        m_socket.close();
      }
    };
    asio::async_read(
        m_socket,
        asio::buffer(m_msgTemporaryIn.data(), m_msgTemporaryIn.size()),
        callback)
  }

  // Once a full message is received, add it to the incoming queue
  void AddToIncomingMessageQueue() {
    // Shove it in queue, converting it to an "owned message", by initialising
    // with the a shared pointer from this connection object
    if (m_nOwnerType == owner::server)
      m_qMessagesIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
    else
      m_qMessagesIn.push_back({nullptr, m_msgTemporaryIn});

    // We must now prime the asio context to receive the next message. It
    // wil just sit and wait for bytes to arrive, and the message construction
    // process repeats itself. Clever huh?
    ReadHeader();
  }

 protected:
  // Each connection has a unique socket to a remote
  asio::ip::tcp::socket m_socket;

  // This context is shared with the whole asio instance
  asio::io_context& m_asioContext;

  // This queue holds all messages to be sent to the remote side
  // of this connection
  tsq_t m_qMessagesOut;

  // This references the incoming queue of the parent object
  tsqueue<owned_message<T>>& m_qMessagesIn;

  // Incoming messages are constructed asynchronously, so we will
  // store the part assembled message here, until it is ready
  size_t in_msg_size = 0;
  msg_t m_msgTemporaryIn;

  // The "owner" decides how some of the connection behaves
  owner m_nOwnerType = owner::server;

  uint32_t id = 0;
};