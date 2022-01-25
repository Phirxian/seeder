#pragma once

#include "Seeder.h"

namespace network
{
    class SeederClient : public Seeder
    {
        public:
            SeederClient(SeederListener *listener = nullptr) noexcept;
            virtual ~SeederClient() noexcept;

            //! initialise and bind socket
            virtual int init(const SocketInfo&) noexcept;
            
            //! connect to localhost ( 127.0.0.1 )
            virtual int connectLocalHost() noexcept;
            //! connect to
            virtual int connectTo(const std::string &ip) noexcept;

            //! reconnect to current context
            virtual int reConnect(const unsigned int retry = 10) noexcept;
            //! reconnect to new ip
            virtual int reConnectTo(const std::string &ip, const unsigned int retry = 10) noexcept;

            //! send data to server
            virtual int sendData(const SendingMessage&) noexcept;

            //! asynchrone waiting data from all peer
            virtual void run() noexcept;
        
        protected:
            std::string ipConnect;
    };
}

