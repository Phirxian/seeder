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

#include "SeederServer.h"

#include <iostream>
#include <errno.h>
#include <string.h>

using namespace std;
using namespace network::option;

namespace network
{
    SeederServer::SeederServer(SeederListener *listener) : Seeder(listener)
    {
    }
    
    SeederServer::~SeederServer()
    {
        while(peer.size())
            deleteClient(peer.size());
        peer.clear();
    }
    
    int SeederServer::init(const SocketInfo &info) noexcept
    {
        errno = 0;
        sem = info;
        sock = socket(AF_INET, sem.mode, sem.type);

        if(errno != 0) // if(sock == SOCKET_ERROR)
        {
            std::cout << "Seeder is not ready (" << errno << "): ";
            std::cout << strerror(errno) << std::endl;
            return errno;
        }

        // prevent timeout
        setupTimeout(sem.timeout);
        
        sync->sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sync->sin.sin_family = AF_INET;
        sync->sin.sin_port = htons(sem.port);

        if(bind(sock, reinterpret_cast<sockaddr *>(&sync->sin), recv_size) < 0)
        {
            std::cout << "Seeder error on bind:" << std::endl;
            std::cout << strerror(errno) << std::endl;
            return errno;
        }
        
        if(listen(sock, 500) == -1)
        {
            std::cout << "Seeder error on listen: " << std::endl;
            std::cout << strerror(errno) << std::endl;
            return errno;
        }

        std::cout << "Seeder is ready : app server, listen " << sem.port << std::endl;
        IsConnected = true;

        return errno;
    }
    
    int SeederServer::sendData(const SendingMessage &msg) noexcept
    {
        if(!msg.data || !msg.length)
            return -1;

        for(unsigned int i=0; i<peer.size();++i)
            sendDataTo(msg, peer[i]);

        return msg.length;
    }
    
    int SeederServer::sendDataTo(const SendingMessage &msg, const unsigned int n_clt) noexcept
    {
        return sendDataTo(msg, peer[n_clt]);
    }
    
    int SeederServer::sendDataTo(const SendingMessage &msg, const ConnectedClient *clt) noexcept
    {
        if(!clt || !clt->sock || !msg.data || !msg.length)
            return -1;

        if(send(clt->sock, (const char*)msg.data, msg.length, MSG_NOSIGNAL|msg.flags) < 0 && listener)
            listener->notifySendError(msg, errno);

        return msg.length;
    }
    
    int SeederServer::sendDataExcepted(const SendingMessage &msg, const unsigned n_clt) noexcept
    {
        return sendDataExcepted(msg, peer[n_clt]);
    }
    
    int SeederServer::sendDataExcepted(const SendingMessage &msg, const ConnectedClient *clt) noexcept
    {
        if(!msg.data || !msg.length)
            return -1;

        for(unsigned int i = 0; i < peer.size();  ++i)
            if(peer[i] != clt)
                sendDataTo(msg, peer[i]);

        return msg.length;
    }
    
    unsigned int SeederServer::getNumberClient() const noexcept
    {
        return peer.size();
    }
    
    void SeederServer::deleteClient(const unsigned int ss) noexcept
    {
        if(ss > peer.size())
            return;
            
        //shutdown(peer[ss]->sock, 2);
        if(closesocket(peer[ss]->sock) == -1)
            std::cout << "error when closing client socket" << std::endl;
            
        delete peer[ss];
        peer.erase(peer.begin()+ss);
    }
    
    void SeederServer::deleteClient(ConnectedClient *cl) noexcept
    {
        deleteClient(findClient(cl));
    }
    
    ConnectedClient* SeederServer::getClient(const unsigned int i) const noexcept
    {
        return peer[i];
    }
    
    unsigned int SeederServer::findClient(const ConnectedClient *cl) const noexcept
    {
        for(unsigned int i = 0; i<peer.size(); ++i)
            if(cl->sock == peer[i]->sock) return i;
        return -1;
    }
    
    ConnectedClient* SeederServer::acceptClient() noexcept
    {
        if(!authoriseAccept)
            return nullptr;
            
        ConnectedClient *client = new ConnectedClient();
        #if defined LINUX
            if((client->sock = accept (sock, (SOCKADDR *) &sync->sin,(socklen_t*)&recv_size)) <= 1) // == INVALID_SOCKET
        #else
            if((client->sock = (SOCKET)accept(sock, (SOCKADDR *) &sync->sin,(int*)&recv_size)) <= 1) // == INVALID_SOCKET
        #endif
        {
            if(listener)
                listener->notifyPeerCrashed(client, -1);
            closesocket(client->sock);
            delete client;
        }
        else
        {
            client->ip = inet_ntoa(sync->sin.sin_addr);
            peer.push_back(client);
            if(listener)
                listener->notifyPeerConnected(client, peer.size());
        }
        
        return client;
    }
    
    void SeederServer::readClient(ConnectedClient *current_peer, int index) noexcept
    {
        ReceivedMessage msg(mtu);
        msg.length = recv(current_peer->sock, msg.buffer, mtu, 0);

        if(msg.length > mtu || msg.length < 0)
        {
            if(listener)
                listener->notifyPeerCrashed(current_peer, index);
            deleteClient(index);
        }
        else if(!msg.length)
        {
            if(listener)
                listener->notifyPeerDisconnected(current_peer, index);
            deleteClient(index);
        }
        else
        {
            msg.recvSockId = index;
            msg.recvSock = current_peer;
            if(listener)
                listener->recvData(msg);
        }
    }
    
    void SeederServer::run() noexcept
    {
        FD_ZERO(&sync->readfs);
        FD_SET(sock, &sync->readfs);
        
        for(unsigned int i = 0; i<peer.size(); ++i)
            FD_SET(peer[i]->sock, &sync->readfs);
            
        int code = select(FD_SETSIZE, &sync->readfs, NULL, NULL, &sync->timeout);

        if(code <= 0)
            return;
            
        if(FD_ISSET(sock, &sync->readfs))
            acceptClient();
        else
        {
            for(unsigned int i = 0; i<peer.size(); ++i)
                if(FD_ISSET(peer[i]->sock, &sync->readfs))
                    readClient(peer[i], i);
        }
    }
}
