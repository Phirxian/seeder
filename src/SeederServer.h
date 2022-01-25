#pragma once

#include "Seeder.h"
#include <vector>

namespace network
{
    class SeederServer : public Seeder
    {
        public:
            typedef std::vector<ConnectedClient*> ClientArray;
        public:
            SeederServer(SeederListener *listener = nullptr);
            ~SeederServer();
            
            //! initialise and bind socket
            int init(const SocketInfo &info) noexcept;
            
            //! allow peer to connect here
            virtual void activeAccept(const bool acpt) noexcept { autoriseAccept = acpt; };
            //! wait new client
            virtual ConnectedClient* acceptClient() noexcept;
            
            //! disconnect peer with ss = peer array position
            virtual void deleteClient(const unsigned int ss) noexcept;
            //! disconnect peer
            virtual void deleteClient(ConnectedClient *cl) noexcept;
            //! get peer position in array from peer data
            virtual unsigned int findClient(const ConnectedClient *cl) const noexcept;
            //! get peer with peer array position
            virtual ConnectedClient* getClient(const unsigned int) const noexcept;
            //! get number of connected client
            virtual unsigned int getNumberClient() const noexcept;

            //! send data to all peer
            virtual int sendData(const SendingMessage&) noexcept;
            //! send data to specified connected peer
            virtual int sendDataTo(const SendingMessage&, const unsigned int n_clt) noexcept;
            //! send data to specified connected peer
            virtual int sendDataTo(const SendingMessage&, const ConnectedClient *clt) noexcept;
            //! send data to all peer exepted it
            virtual int sendDataExcepted(const SendingMessage&, const unsigned n_clt) noexcept;
            //! send data to all peer exepted it
            virtual int sendDataExcepted(const SendingMessage&, const ConnectedClient *clt) noexcept;

            //! asynchrone waiting data from all peer
            virtual void run() noexcept;
            
        protected:
            //! process one client, recv or check connection status
            virtual void readClient(ConnectedClient *current_peer, int index) noexcept;
            
        protected:
            //! allow new clients
            bool autoriseAccept;
            //! list of connected clients
            ClientArray peer;
    };
}

