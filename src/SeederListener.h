#pragma once

#include "SeederConfig.h"
#include "SeederMessage.h"

namespace network
{
    struct ConnectedClient
    {
        ConnectedClient() :
            sock(0), ip("0.0.0.0"),
            mac("00:00:00:00:00:00:00:00"), user(0)
        {
        }

        SEEDER_SOCKET sock;
        const char *ip;
        const char *mac;

        void *user;
    };
    
    class SeederListener
    {
        public:
            virtual ~SeederListener(){}
            virtual void notifySendError(const SendingMessage&, int error) = 0;
            virtual void notifySendError(const UnconnectedMessage&, int error) = 0;

            //! server side
            virtual void notifyPeerLost(ConnectedClient*, int index) = 0;
            virtual void notifyPeerCrashed(ConnectedClient*, int index) = 0;
            virtual void notifyPeerConnected(ConnectedClient*, int index) = 0;
            virtual void notifyPeerDisconnected(ConnectedClient*, int index) = 0;

            //! client side
            virtual void notifyConnected(bool) = 0;
            virtual void notifyConnectionLost() = 0;
            
            //! server and client
            virtual void recvData(const ReceivedMessage &msg) = 0;
    };
}
