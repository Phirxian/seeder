#pragma once

#include "SeederConfig.h"

namespace network
{
    class SeederListener
    {
        public:
            virtual ~SeederListener(){}
            virtual void notifySendError(const std::string &msg, int error, int flags, const option::Client *c = 0) = 0;

            //! server side
            virtual void notifyPeerLost(option::Client*, int index) = 0;
            virtual void notifyPeerCrashed(option::Client*, int index) = 0;
            virtual void notifyPeerConnected(option::Client*, int index) = 0;
            virtual void notifyPeerDisconnected(option::Client*, int index) = 0;

            virtual void recvData(option::Client *c, const std::string &msg) = 0;

            //! client side
            virtual void notifyConnected(bool) = 0;
            virtual void notifyConnectionLost() = 0;

            virtual void recvData(const std::string &msg) = 0;
    };
}
