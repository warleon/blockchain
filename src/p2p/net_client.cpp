#pragma once

#include "net_common.h"
#include "net_tsqueue.cpp"
#include "net_message.cpp"


namespace olc
{
    namespace net
    {
        template<typename T>
        class client_interface
        {
        public:
            client_interface()
                : m_socket(m_asioContext)
            {}

            virtual ~client_interface()
            {
                Disconnect();
            }

            bool Connect(const std::string& host, const uint16_t port, const uint32_t uid = 0)
            {
                try
                {
                    m_socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));

                    m_threadContext = std::thread([this]() { m_asioContext.run(); });
                }
                catch (std::exception& e)
                {
                    std::cerr << "[CLIENT] Exception: " << e.what() << std::endl;
                    return false;
                }

                m_connection = std::make_unique<connection<T>>(m_asioContext, std::move(m_socket), m_qMessagesIn);
                m_connection->ConnectToPeer(m_resolver.resolve(host, std::to_string(port)), uid);

                std::cout << "[CLIENT] Connected to Server!\n";
                return true;
            }

            void Disconnect()
            {
                if (IsConnected())
                {
                    m_connection->Disconnect();
                    m_connection.reset();
                }

                m_asioContext.stop();

                if (m_threadContext.joinable())
                    m_threadContext.join();

                std::cout << "[CLIENT] Disconnected from Server!\n";
            }

            bool IsConnected()
            {
                if (m_connection)
                    return m_connection->IsConnected();
                else
                    return false;
            }

            void Send(const message<T>& msg)
            {
                if (IsConnected())
                    m_connection->Send(msg, 0);
            }

            void Update(size_t nMaxMessages = -1)
            {
                size_t nMessageCount = 0;
                while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
                {
                    auto msg = m_qMessagesIn.pop_front();
                    OnMessage(msg);
                    nMessageCount++;
                }
            }

        protected:
            virtual void OnMessage(message<T>& msg)
            {}

        protected:
            asio::io_context m_asioContext;
            std::thread m_threadContext;
            asio::ip::tcp::socket m_socket;
            std::unique_ptr<connection<T>> m_connection;
            tsqueue<message<T>> m_qMessagesIn;
            asio::ip::tcp::resolver m_resolver;
        };
    }
}
