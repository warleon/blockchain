#pragma once

#include "net_common.h"
#include "net_tsqueue.cpp"
#include "net_message.cpp"


namespace olc
{
    namespace net
    {
        template<typename T>
        class connection : public std::enable_shared_from_this<connection<T>>
        {
        public:
            enum class owner
            {
                none,
                self,
                other
            };

        public:
            connection(asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
                : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
            {
                m_nOwnerType = owner::none;
            }

            virtual ~connection()
            {}

            uint32_t GetID() const
            {
                return id;
            }

        public:
            void ConnectToPeer(asio::ip::tcp::resolver::results_type& endpoints, uint32_t uid = 0)
            {
                if (m_socket.is_open())
                {
                    id = uid;
                    m_nOwnerType = owner::self;
                    ReadHeader();
                }
            }

            void Disconnect()
            {
                if (IsConnected())
                    asio::post(m_asioContext, [this]() { m_socket.close(); });
            }

            bool IsConnected() const
            {
                return m_socket.is_open();
            }

            void StartListening()
            {
                if (m_nOwnerType == owner::self)
                    ReadHeader();
            }

        public:
            void Send(const message<T>& msg, uint32_t targetID)
            {
                asio::post(m_asioContext,
                    [this, msg, targetID]()
                    {
                        bool bWritingMessage = !m_qMessagesOut.empty();
                        msg.targetID = targetID;
                        m_qMessagesOut.push_back(msg);
                        if (!bWritingMessage)
                        {
                            WriteHeader();
                        }
                    });
            }

        private:
            void WriteHeader()
            {
                asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
                    [this](std::error_code ec, std::size_t length)
                    {
                        if (!ec)
                        {
                            if (m_qMessagesOut.front().body.size() > 0)
                            {
                                WriteBody();
                            }
                            else
                            {
                                m_qMessagesOut.pop_front();
                                if (!m_qMessagesOut.empty())
                                {
                                    WriteHeader();
                                }
                            }
                        }
                        else
                        {
                            std::cout << "[" << id << "] Write Header Fail.\n";
                            m_socket.close();
                        }
                    });
            }

            void WriteBody()
            {
                asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                    [this](std::error_code ec, std::size_t length)
                    {
                        if (!ec)
                        {
                            m_qMessagesOut.pop_front();
                            if (!m_qMessagesOut.empty())
                            {
                                WriteHeader();
                            }
                        }
                        else
                        {
                            std::cout << "[" << id << "] Write Body Fail.\n";
                            m_socket.close();
                        }
                    });
            }

            void ReadHeader()
            {
                asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
                    [this](std::error_code ec, std::size_t length)
                    {
                        if (!ec)
                        {
                            if (m_msgTemporaryIn.header.size > 0)
                            {
                                m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                ReadBody();
                            }
                            else
                            {
                                AddToIncomingMessageQueue();
                            }
                        }
                        else
                        {
                            std::cout << "[" << id << "] Read Header Fail.\n";
                            m_socket.close();
                        }
                    });
            }

            void ReadBody()
            {
                asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                    [this](std::error_code ec, std::size_t length)
                    {
                        if (!ec)
                        {
                            AddToIncomingMessageQueue();
                        }
                        else
                        {
                            std::cout << "[" << id << "] Read Body Fail.\n";
                            m_socket.close();
                        }
                    });
            }

            void AddToIncomingMessageQueue()
            {
                if (m_nOwnerType == owner::self)
                    m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
                else
                    m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

                ReadHeader();
            }

        protected:
            asio::ip::tcp::socket m_socket;
            asio::io_context& m_asioContext;
            tsqueue<message<T>>& m_qMessagesOut;
            tsqueue<owned_message<T>>& m_qMessagesIn;
            message<T> m_msgTemporaryIn;
            owner m_nOwnerType = owner::none;
            uint32_t id = 0;
        };
    }
}
