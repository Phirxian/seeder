#include <errno.h>
#include <stdlib.h>

#ifdef __WIN32
#   include <winsock2.h>
#   include <unistd.h>
    typedef int socklen_t;
#else
#   include <sys/types.h>
#   include <netdb.h>
#   include <sys/socket.h>
#   include <sys/ioctl.h>
#endif

#include "SeederClient.h"

#include <iostream>
#include <errno.h>
#include <string.h>

using namespace std;
using namespace network::option;

namespace network
{
    SeederClient::SeederClient(SeederListener *listener) noexcept : Seeder(listener)
    {
    }
    
    SeederClient::~SeederClient() noexcept
    {
    }
    
    int SeederClient::init(const SocketInfo &info) noexcept
    {
        errno = 0;
        sem = info;
        IsConnected = false;

        sock = socket(AF_INET, sem.mode, sem.type);

        if(errno != 0) // if(sock == SOCKET_ERROR)
        {
            std::cout << "Seeder is not ready (" << errno << "): ";
            std::cout << strerror(errno) << std::endl;
            return errno;
        }

        // prevent timeout
        setupTimeout(sem.timeout);
        std::cout << "Seeder is ready : app client" << std::endl;

        return errno;
    }

    int SeederClient::connectLocalHost() noexcept
    {
        ipConnect = "localhost";
        sync->sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sync->sin.sin_family      = AF_INET;
        sync->sin.sin_port        = htons (sem.port);

        int error = connect(sock, (SOCKADDR *)&sync->sin, sizeof(sync->sin));
        IsConnected = (error == 0);

        std::cout << "connected on server : IsConnected "
                  << IsConnected
                  << " error "
                  << error
                  << std::endl;

        if(listener)
            listener->notifyConnected(IsConnected);
        
        return errno;
    }

    int SeederClient::connectTo(const std::string &ip) noexcept
    {
        ipConnect = ip;

        struct hostent *hp = gethostbyname(ip.c_str());
        if(hp) memcpy((char *)&sync->sin.sin_addr.s_addr, (char*)hp->h_addr, hp->h_length);
        else sync->sin.sin_addr.s_addr = inet_addr(ip.c_str());

        sync->sin.sin_family      = AF_INET;
        sync->sin.sin_port        = htons(sem.port);

        errno = 0;
        int error = connect(sock, (SOCKADDR *)&sync->sin, sizeof(sync->sin));

        #ifndef __WIN32
        if(error != 0 && errno == EINPROGRESS)
        #else
        if(WSAGetLastError() == WSAEINPROGRESS)
        #endif
        {
            fd_set write_fd;
            FD_ZERO(&write_fd);
            FD_SET(sock,&write_fd);

            if(0 < select(sock , NULL,&write_fd,NULL,&sync->timeout))
            {
                int err = 0;
                socklen_t errlen = sizeof(err);
                error = getsockopt(
                    sock, SOL_SOCKET, SO_ERROR,
                    (sockopt)(&err), &errlen
                );
                error = (0 <= error) * ECONNREFUSED;
            }
            else error = ETIMEDOUT;
        }

        if(error == 0)
        {
            std::cout << "connected to server : " << ip.c_str() << ":" << sem.port << std::endl;
            IsConnected = true;
        }
        else
        {
            std::cout << "an error as occurred will connecting to " << ip.c_str() << " : " << sem.port << std::endl;
            if(hp)
            {
                struct in_addr *addr = (struct in_addr*)hp->h_addr;
                std::cout << "(" << (char*)inet_ntoa(*addr) << ")" << std::endl;
            }
            std::cout << strerror(errno) << std::endl;
            IsConnected = false;
        }

        if(listener)
            listener->notifyConnected(IsConnected);
        return error == 0 ? true : false;
    }
    
    int SeederClient::reConnectTo(const std::string &ip, const unsigned int retry) noexcept
    {
        shutdown();
        init(sem);
        
        for(unsigned char i = 0; i<retry; ++i)
        {
            #if defined WINDOWS
                Sleep(35);
            #else
                usleep(35*1000);
            #endif
            
            int t = connectTo(ip);
            if(isConnected())
                return t;
        }
        return -1;
    }
    
    int SeederClient::reConnect(const unsigned int retry) noexcept
    {
        return reConnectTo(ipConnect, retry);
    }
    
    int SeederClient::sendData(const SendingMessage &msg) noexcept
    {
        if(!IsConnected || !msg.data|| !msg.length)
            return -1;

        if(send(sock, (const char*)msg.data, msg.length, MSG_NOSIGNAL|msg.flags) < 0)
            if(listener) listener->notifySendError(msg, errno);

        return errno;
    }
    
    void SeederClient::run() noexcept
    {
        FD_ZERO(&sync->readfs);
        FD_SET(sock, &sync->readfs);
        
        int code = select(FD_SETSIZE, &sync->readfs, NULL, NULL, &sync->timeout);

        if(code <= 0 || !FD_ISSET(sock, &sync->readfs))
            return;
            
        ReceivedMessage msg = waitRecvData();

        if(msg.length > mtu || msg.length <= 0)
        {
            if(listener)
                listener->notifyConnectionLost();
            shutdown();
        }
        else
            listener->recvData(msg);
    }
}
