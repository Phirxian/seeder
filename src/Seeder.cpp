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

#include "Seeder.h"

#include <iostream>
#include <errno.h>
#include <string.h>

using namespace std;
using namespace network::option;

namespace network
{
    Seeder::Seeder(SeederListener*listener) noexcept :
        listener(listener),
        sync(new SyncNetwork())
    {
        setMTU(1492);
        #if defined (WIN32)
            WSADATA WSAData;
            if(WSAStartup(MAKEWORD(2,0), &WSAData))
                  std::cout << "Can't not init network service !" << std::endl;
            else  std::cout << "Network service as be init\n" << std::endl;
        #endif
    }
    
    Seeder::~Seeder() noexcept
    {
        #if defined (WIN32)
            if(!WSACleanup())
                std::cout << "Can't not end network service !" << std::endl;
        #endif
        closesocket(sock);
    }
    
    int Seeder::shutdown() noexcept
    {
        IsConnected = false;
        //shutdown(sock, 1);
        return closesocket(sock)+1;
    }

    void Seeder::setupTimeout(const int ms) noexcept
    {
        recsize = (int)sizeof(sync->sin);
        sync->timeout.tv_sec = 0;
        sync->timeout.tv_usec = ms*1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&sync->timeout, sizeof(sync->timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&sync->timeout, sizeof(sync->timeout));

        int option = 1;
        setsockopt(sock, SOL_SOCKET, SO_DEBUG, (char*)&option, sizeof(option));

//        option = 1;
//        ioctl(sock, FIONBIO, (char*) &option);

        option = 1;
        ioctl(sock, FIOASYNC, (char*)&option);

        // re-use adresse&write_fd
        option = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int));

        //prevent SIGPIPE = 0x0800 = SO_NOSIGPIPE
        #ifdef SO_NOSIGPIPE
            int set = 1;
            setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
        #endif

    }
    
    ReceivedMessage Seeder::waitRecvData() noexcept
    {
        ReceivedMessage msg(mtu);
        msg.length = recv(sock, msg.buffer, mtu, 0);
        return msg;
    }
    
    int Seeder::sendDataTo(const UnconnectedMessage &msg) noexcept
    {
        if(!msg.buffer || !msg.length || !msg.ip)
            return -1;
        
        SOCKADDR_IN peer;
        peer.sin_family = AF_INET;
        peer.sin_port = htons(msg.port);

        struct hostent *hp = gethostbyname(msg.ip);
        
        if(hp) memcpy((char *)&peer.sin_addr.s_addr, (char*)hp->h_addr, hp->h_length);
        else peer.sin_addr.s_addr = inet_addr(msg.ip);

        if(sendto(sock, (const char*)msg.buffer, msg.length, MSG_NOSIGNAL|msg.flags, (struct sockaddr*)&peer, sizeof(peer)) < 0)
            if(listener) listener->notifySendError(msg, errno);

        return errno;
    }
    
    UnconnectedMessage Seeder::waitRecvDataFrom(const char*ip, const int port) noexcept
    {
        SOCKADDR_IN peer;
        peer.sin_family = AF_INET; // using IPV4
        peer.sin_port = htons(port);

        struct hostent *hp = gethostbyname(ip);
        if(hp) memcpy((char *)&peer.sin_addr.s_addr, (char*)hp->h_addr, hp->h_length);
        else peer.sin_addr.s_addr = inet_addr(ip);

        int tempo = sizeof(peer);
        
        UnconnectedMessage msg(mtu);
        msg.ip = ip;
        msg.port = port;
        
        #if defined LINUX
            msg.length = recvfrom(sock,msg.buffer,mtu,0,(struct sockaddr*)&peer,(socklen_t*)&tempo);
        #else
            msg.length = recvfrom(sock,msg.buffer,mtu,0,(struct sockaddr*)&peer,&tempo);
        #endif
        
        return msg;
    }
    
    SEEDER_SOCKET& Seeder::getSocket() noexcept
    {
        return sock;
    }
    
    Seeder::SyncNetwork* Seeder::getSyncUtil() noexcept
    {
        return sync;
    }
    
    SocketInfo& Seeder::getInfo() noexcept
    {
        return sem;
    }
}