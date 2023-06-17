#pragma once
#include "net_common.h"

namespace olc{
    namespace net{
        template<typename T>
        class tsqueue{
            public:
            tsqueue() = default;
            tsqueue(const tsqueue<T>&) = delete;
            virtual ~tsqueue() {clear();}
            protected:
            std::mutex muxQueue;
            std::deque<T> deqQueue;

            public:

            const T& front(){
                std::scoped_lock lock(muxQueue);
                return deqQueue.front();
            }

            const T& back(){
                std::scoped_lock lock(muxQueue);
                return deqQueue.front();
            }

            void push_back(const T& item){
                std::scoped_lock lock(muxQueue);
                deqQueue.emplace_back(std::move(item));
            }

            void push_front(const T& item){
                std::scoped_lock lock(muxQueue);
                deqQueue.emplace_front(std::move(item));
            }

            bool empty(){
                std::scoped_lock lock(muxQueue);
                return deqQueue.size();
            }

            size_t size(){
                std::scoped_lock lock(muxQueue);
                return deqQueue.size();
            }

            void clear(){
                std::scoped_lock lock(muxQueue);
                deqQueue.clear();
            }

            T pop_front(){
                std::scoped_lock lock(muxQueue);
                auto t = std::move(deqQueue.front());
                deqQueue.pop_front();
                return t;
            }

            T pop_back(){
                std::scoped_lock lock(muxQueue);
                auto t = std::move(deqQueue.front());
                deqQueue.pop_back();
                return t;
            }

        };


        template <typename T>
        class connection;
        
        template <typename T>
        struct owned_message{
            std::shared_ptr<connection<T>> remote = nullptr;
            //message<T>;

            friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg){
                os << msg.msg;
                return os;
            }
        };
    }
}