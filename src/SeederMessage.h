#include "SeederListener.h"
#include <string.h>

namespace network
{
    class ConnectedClient;
    
    struct ReceivedMessage
    {
        ReceivedMessage(int mtu = 0)
        {
            buffer = new char[mtu];
            length = 0;
            recvSockId = -1;
            recvSock = nullptr;
        }
        
        ~ReceivedMessage()
        {
            delete[] buffer;
        }
        
        std::string to_string() const noexcept
        {
            return std::string((const char*)buffer, length);
        }
        
        char *buffer;
        //! size of message -1 if SOCKET_ERROR
        int length;
        //! peer emitter index
        unsigned int recvSockId;
        //! peer emitter pointer
        ConnectedClient *recvSock;
    };
    
    struct SendingMessage
    {
        SendingMessage(const void *data, unsigned int length) :
            data(data), length(length), flags(0)
        {
        }
        
        std::string to_string() const noexcept
        {
            return std::string((const char*)data, length);
        }
        
        const void *data;
        unsigned int length;
        int flags;
    };
    
    /**
      UDP Mode, send buffer in unconnected mode
     **/
    struct UnconnectedMessage
    {
        UnconnectedMessage(int mtu) noexcept :
            buffer(new char[mtu]), length(0), ip(nullptr), port(0), flags(0)
        {
        }
        
        UnconnectedMessage(const char *msg, unsigned int size) noexcept :
            ip(nullptr), port(0), flags(0)
        {
            length = size;
            buffer = new char[length];
            memcpy(buffer, msg, length);
        }
        
        UnconnectedMessage(const std::string &msg) noexcept :
            ip(nullptr), port(0), flags(0)
        {
            length = msg.size()+1;
            buffer = new char[length];
            memcpy(buffer, msg.c_str(), length);
        }
        
        ~UnconnectedMessage()
        {
            delete[] buffer;
        }
        
        std::string to_string()
        {
            return std::string((const char*)buffer, length);
        }
        
        char *buffer;
        unsigned int length;
        const char *ip;
        int port;
        int flags;
    };
}