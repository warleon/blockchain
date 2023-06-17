#pragma once

#include "net_common.h"
#include "net_tsqueue.cpp"
#include "net_message.cpp"

namespace olc{
    namespace net{
        template<typename T>
        class server_interface
        {
        public:
            server_interface(uint16_t port)
                : m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
            {}

            virtual ~server_interface()
            {
                Stop();
            }

            bool Start()
            {
                try
                {
                    WaitForClientConnection();

                    m_threadContext = std::thread([this]() { m_asioContext.run(); });
                }
                catch (std::exception& e)
                {
                    std::cerr << "[SERVER] Exception: " << e.what() << std::endl;
                    return false;
                }

                std::cout << "[SERVER] Started!\n";
                return true;
            }

            void Stop()
            {
                m_asioContext.stop();

                if (m_threadContext.joinable())
                    m_threadContext.join();

                std::cout << "[SERVER] Stopped!\n";
            }

            void WaitForClientConnection()
            {
                m_asioAcceptor.async_accept(
                    [this](std::error_code ec, asio::ip::tcp::socket socket)
                    {
                        if (!ec)
                        {
                            uint32_t uid = 0; // Generate unique ID for the client

                            std::shared_ptr<connection<T>> newconn =
                                std::make_shared<connection<T>>(m_asioContext, std::move(socket), m_qMessagesIn);

                            if (OnClientConnect(newconn, uid))
                            {
                                m_deqConnections.push_back(std::move(newconn));
                                m_deqConnections.back()->ConnectToPeer(m_resolver.resolve(socket.remote_endpoint().address().to_string(), "65535"), uid);

                                std::cout << "[" << uid << "] Connection Approved\n";
                            }
                            else
                            {
                                std::cout << "[-----] Connection Denied\n";
                            }
                        }
                        else
                        {
                            std::cout << "[SERVER] New Connection Error: " << ec.message() << std::endl;
                        }

                        WaitForClientConnection();
                    });
            }

            void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
            {
                if (client && client->IsConnected())
                    client->Send(msg, 0);
                else
                    OnClientDisconnect(client);
            }

            void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
            {
                bool bInvalidClientExists = false;

                for (auto& client : m_deqConnections)
                {
                    if (client && client->IsConnected())
                    {
                        if (client != pIgnoreClient)
                            client->Send(msg, 0);
                    }
                    else
                    {
                        OnClientDisconnect(client);
                        bInvalidClientExists = true;
                    }
                }

                if (bInvalidClientExists)
                    m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
            }

            void Update(size_t nMaxMessages = -1)
            {
                size_t nMessageCount = 0;
                while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
                {
                    auto msg = m_qMessagesIn.pop_front();
                    OnMessage(msg.remote, msg.msg);

                    nMessageCount++;
                }
            }

        protected:
            virtual bool OnClientConnect(std::shared_ptr<connection<T>> client, uint32_t uid)
            {
                return false;
            }

            virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
            {}

            virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
            {}

        protected:
            tsqueue<owned_message<T>> m_qMessagesIn;
            std::deque<std::shared_ptr<connection<T>>> m_deqConnections;
            asio::io_context m_asioContext;
            std::thread m_threadContext;
            asio::ip::tcp::acceptor m_asioAcceptor;
            asio::ip::tcp::resolver m_resolver;
        };
    }
}

