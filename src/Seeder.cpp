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
    Seeder::Seeder() noexcept : buffer(0), listener(0), sync(new SyncNetwork()), mtu(0), IsConnected(false)
    {
        setMTU(1492);
        #if defined (WIN32)
            WSADATA WSAData;
            if(WSAStartup(MAKEWORD(2,0), &WSAData))
                  std::cout << "Can't not init network service !" << std::endl;
            else  std::cout << "Network service as be init\n" << std::endl;
        #endif
        sem.autoriseAccept = false;
        sem.PORT = 0;
        end();
    }
    Seeder::~Seeder() noexcept
    {
        exit();
        #if defined (WIN32)
            if(!WSACleanup()) std::cout << "Can't not end network service !" << std::endl;
        #endif
        delete[] buffer;
    }
    void Seeder::setListener(SeederListener *l) noexcept
    {
        listener = l;
    }
    SeederListener* Seeder::getListener() const noexcept
    {
        return listener;
    }
    void Seeder::setMTU(const int i) noexcept
    {
        newmtu = i;
    }
    int Seeder::getMTU() const noexcept
    {
        return mtu;
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
    int Seeder::init(const S_APPLICATION_MODE app, const S_TRANSFAIRE_MODE mode, const int p, const int timeout, option::S_PROTOCOL_MODE type) noexcept
    {
        sem.APP = app;
        sem.MODE = mode;
        sem.PORT = p;

        errno = 0;
        IsConnected = false;

        if(type == option::SPM_AUTO)
        {
            switch(mode)
            {
                case STM_TCP: type = option::SPM_TCP; break;
                case STM_UDP: type = option::SPM_UDP; break;
                case STM_RAW: type = option::SPM_RAW; break;
                default:      type = option::SPM_TCP; break;
            }
        }

        sock = socket(AF_INET, mode, type);

        if(errno != 0) // if(sock == SOCKET_ERROR)
        {
            std::cout << "Seeder is not ready (" << errno << "): ";
            std::cout << strerror(errno) << std::endl;
            return errno;
        }

        // prevent timeout
        setupTimeout(timeout);

        if(app == SAM_SERVEUR)
        {
            sync->sin.sin_addr.s_addr = htonl(INADDR_ANY);
            sync->sin.sin_family = AF_INET;
            sync->sin.sin_port = htons(sem.PORT);

            if(bind(sock, reinterpret_cast<sockaddr *>(&sync->sin), recsize) < 0)
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

            std::cout << "Seeder is ready : app server, listen " << sem.PORT << std::endl;
            IsConnected = true;
        }
        else std::cout << "Seeder is ready : app client" << std::endl;

        return errno;
    }
    int Seeder::connectLocalHost() noexcept
    {
        sem.ipConnect = "localhost";
        sync->sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sync->sin.sin_family      = AF_INET;
        sync->sin.sin_port        = htons (sem.PORT);

        int error = connect(sock, (SOCKADDR *)&sync->sin, sizeof(sync->sin));
        IsConnected = (error == 0);

        std::cout << "connected on server : "
                  << (error < 0 ? errno : 1)
                  << " error"
                  << std::endl;

        if(listener)
            listener->notifyConnected(IsConnected);

        return errno;
    }
    int Seeder::connectTo(const std::string &ip) noexcept
    {
        end();
        sem.ipConnect = ip;

        struct hostent *hp = gethostbyname(ip.c_str());
        if(hp) memcpy((char *)&sync->sin.sin_addr.s_addr, (char*)hp->h_addr, hp->h_length);
        else sync->sin.sin_addr.s_addr = inet_addr(ip.c_str());

        sync->sin.sin_family      = AF_INET;
        sync->sin.sin_port        = htons(sem.PORT);

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
            std::cout << "connected to server : " << ip.c_str() << ":" << sem.PORT << std::endl;
            IsConnected = true;
        }
        else
        {
            std::cout << "an error as occurred will connecting to " << ip.c_str() << " : " << sem.PORT << std::endl;
            if(hp)
            {
                struct in_addr *addr = (struct in_addr*)hp->h_addr;
                std::cout << "(" << (char*)inet_ntoa(*addr) << ")" << std::endl;
            }
            std::cout << strerror(errno) << std::endl;
            IsConnected = false;
        }

        if(listener) listener->notifyConnected(IsConnected);
        return error == 0 ? true : false;
    }
    int Seeder::reConnectTo(const std::string &ip, const unsigned int retry) noexcept
    {
        end();
        shutdown();
        init(sem.APP, sem.MODE, sem.PORT, sync->timeout.tv_usec/1000, sem.TYPE);
        for(unsigned char i = 0; i<retry; ++i)
        {
            #if defined WINDOWS
                Sleep(35);
            #else
                usleep(35*1000);
            #endif
            int t = connectTo(ip);
            if(isConnected()) return t;
        }
        return -1;
    }
    int Seeder::reConnect(const unsigned int retry) noexcept
    {
        return reConnectTo(sem.ipConnect, retry);
    }
    int Seeder::sendData(const void *data, const unsigned int length, int flags) noexcept
    {
//        printf("all >> %s\n", data);
        if(!IsConnected || !data || !length) return -1;

        if(sem.APP == SAM_CLIENT)
        {
            if(send(sock, (const char*)data,length,MSG_NOSIGNAL|flags) < 0)
                if(listener) listener->notifySendError("", errno, flags, 0);
        }
        else
        {
            for(unsigned int i=0; i<peer.size();++i)
                sendDataTo(data,length,i,flags);
        }

        sem.error = errno;
        return sem.error;
    }
    int Seeder::sendDataTo(const void *data, const unsigned int lenght, const unsigned int n_clt, int flags) noexcept
    {
//        printf("%d >> %s\n", n_clt, data);
        if(!IsConnected || !data || !lenght)
            return -1;

        if(sem.APP == SAM_CLIENT)
        {
            if(send(sock, (const char*)data,lenght, MSG_NOSIGNAL|flags) < 0)
                if(listener) listener->notifySendError("", errno, flags, peer[n_clt]);
        }
        else
        {
            if(send(peer[n_clt]->sock, (const char*)data,lenght,MSG_NOSIGNAL|flags) < 0)
                if(listener) listener->notifySendError("", errno, flags, peer[n_clt]);
        }

        sem.error = errno;
        return sem.error;
    }
    int Seeder::sendDataTo(const void *data, const unsigned int length, const option::Client *clt, int flags) noexcept
    {
//        printf("%s:%d >> %s\n", clt->ip, clt->sock, data);
        if(!IsConnected || !clt || !clt->sock || !data || !length)
            return -1;

        if(sem.APP == SAM_CLIENT)
        {
            if(send(sock, (const char*)data,length,MSG_NOSIGNAL|flags) < 0 && listener)
                listener->notifySendError("", errno, flags, clt);
        }
        else
        {
            if(send(clt->sock, (const char*)data,length,MSG_NOSIGNAL|flags) < 0 && listener)
                listener->notifySendError("", errno, flags, clt);
        }

        sem.error = errno;
        return sem.error;
    }
    int Seeder::sendDataExcepted(const void *data, const unsigned int lenght, const unsigned n_clt, int flags) noexcept
    {
//        printf("%d exepted >> %s\n", n_clt, data);
        if(!IsConnected || !data || !lenght)
            return -1;

        for(unsigned int i = 0; i < peer.size();  ++i)
            if(i != n_clt)
                sendDataTo(data,lenght, peer[i],flags);

        return lenght;
    }
    int Seeder::sendDataExcepted(const void *data, const unsigned int lenght, const option::Client *clt, int flags) noexcept
    {
        return sendDataExcepted(data, lenght, findClient(clt),flags);
    }
    int Seeder::sendDataTo(const void *data, const unsigned int length, const char*ip, const int port, int flags) noexcept
    {
        if(!data || !length || !ip) return -1;
//        printf("%s:%d >> %s\n", ip, port, data);
        SOCKADDR_IN peer;
        peer.sin_family = AF_INET;
        peer.sin_port = htons(port);

        struct hostent *hp = gethostbyname(ip);
        if(hp) memcpy((char *)&peer.sin_addr.s_addr, (char*)hp->h_addr, hp->h_length);
        else peer.sin_addr.s_addr = inet_addr(ip);

        if(sendto(sock, (const char*)data,length, MSG_NOSIGNAL|flags,(struct sockaddr*)&peer,sizeof(peer)) < 0)
            if(listener) listener->notifySendError("", errno, flags);

        sem.error = errno;
        return sem.error;
    }
    int Seeder::sendMsg(const std::string &msg, int flags) noexcept
    {
        return sendData((void*)msg.c_str(), msg.size()+1,flags);
    }
    int Seeder::sendMsgTo(const std::string &msg, const unsigned int n_clt, int flags) noexcept
    {
        return sendDataTo((void*)msg.c_str(), msg.size()+1, n_clt,flags);
    }
    int Seeder::sendMsgTo(const std::string &msg, const option::Client *n_clt, int flags) noexcept
    {
        return sendDataTo((void*)msg.c_str(), msg.size()+1, n_clt,flags);
    }
    int Seeder::sendMsgExcepted(const std::string &msg, const unsigned int n_clt, int flags) noexcept
    {
        return sendDataExcepted((void*)msg.c_str(), msg.size()+1, n_clt,flags);
    }
    int Seeder::sendMsgExcepted(const std::string &msg, const option::Client *n_clt, int flags) noexcept
    {
        return sendDataExcepted((void*)msg.c_str(), msg.size()+1, n_clt,flags);
    }
    int Seeder::sendMsgTo(const std::string &msg, const char *ip, const int port, int flags) noexcept
    {
        return sendDataTo((void*)msg.c_str(), msg.size()+1, ip, port,flags);
    }
    std::string Seeder::recvData() const noexcept
    {
        return sem.octRecv < mtu ? std::string(buffer, sem.octRecv) : "\u0004";
    }
    std::string Seeder::waitRecvData() noexcept
    {
        clearBuffer(0);
        sem.octRecv = recv(sock,buffer,mtu,0);
        return std::string(buffer, sem.octRecv <= mtu ? sem.octRecv : mtu);
    }
    std::string Seeder::waitRecvDataFrom(const char*ip, const int port) noexcept
    {
        clearBuffer(0);
        SOCKADDR_IN peer;
        peer.sin_family = AF_INET; // using IPV4
        peer.sin_port = htons(port);

        struct hostent *hp = gethostbyname(ip);
        if(hp) memcpy((char *)&peer.sin_addr.s_addr, (char*)hp->h_addr, hp->h_length);
        else peer.sin_addr.s_addr = inet_addr(ip);

        int tempo = sizeof(peer);
        #if defined LINUX
            sem.octRecv = recvfrom(sock,buffer,mtu,0,(struct sockaddr*)&peer,(socklen_t*)&tempo);
        #else
            sem.octRecv = recvfrom(sock,buffer,mtu,0,(struct sockaddr*)&peer,&tempo);
        #endif
        return std::string(buffer, sem.octRecv <= mtu ? sem.octRecv : mtu);
    }
    SEEDER_SOCKET& Seeder::getSocket() noexcept
    {
        return sock;
    }
    Seeder::SyncNetwork* Seeder::getSyncUtil() noexcept
    {
        return sync;
    }
    option::SocketInfo& Seeder::getInfo() noexcept
    {
        return sem;
    }
    unsigned int Seeder::getNumberClient() const noexcept
    {
        return peer.size();
    }
    void Seeder::run() noexcept
    {
        FD_ZERO(&sync->readfs);
        FD_SET(sock, &sync->readfs);
        for (unsigned int i = 0; i<peer.size(); ++i) FD_SET(peer[i]->sock, &sync->readfs);
        int code = select(FD_SETSIZE, &sync->readfs, NULL, NULL, &sync->timeout);

        switch(code)
        {
            case -1: case 0: break;
            default:
            if(sem.APP == SAM_CLIENT)
            {
                if(FD_ISSET(sock, &sync->readfs))
                {
                    sem.octRecv = recv(sock,buffer,mtu,0);

                    if(sem.octRecv > mtu || sem.octRecv <= 0)
                    {
                        if(listener)
                            listener->notifyConnectionLost();
                        shutdown();
                    }
                    else
                    {
                        if(listener)
                            listener->recvData(recvData());
                    }
                }
            }
            else if(sem.APP == SAM_SERVEUR)
            {
                if(FD_ISSET(sock, &sync->readfs))
                {
                    if(sem.autoriseAccept)
                    {
                        option::Client *NewConnection = new option::Client();
                        #if defined LINUX
                            if((NewConnection->sock = accept (sock, (SOCKADDR *) &sync->sin,(socklen_t*)&recsize)) <= 1) // == INVALID_SOCKET
                        #else
                            if((NewConnection->sock = (SOCKET)accept(sock, (SOCKADDR *) &sync->sin,(int*)&recsize)) <= 1) // == INVALID_SOCKET
                        #endif
                        {
                            if(listener) listener->notifyPeerCrashed(NewConnection, -1);
                            closesocket(NewConnection->sock);
                            delete NewConnection;
                        }
                        else
                        {
                            NewConnection->ip = inet_ntoa(sync->sin.sin_addr);
                            peer.push_back(NewConnection);
                            if(listener)
                                listener->notifyPeerConnected(NewConnection, peer.size());
                        }
                    }
                }
                else
                {
                    for(unsigned int i = 0; i<peer.size(); ++i)
                    {
                        if(FD_ISSET(peer[i]->sock,&sync->readfs))
                        {
                            sem.octRecv = recv(peer[i]->sock,buffer,mtu,0);

                            //! review this part (potential)
                            if(sem.octRecv > mtu || sem.octRecv < 0)
                            {
                                if(listener)
                                    listener->notifyPeerCrashed(peer[i], i);
                                deleteClient(i);
                            }
                            else if(!sem.octRecv)
                            {
                                if(listener)
                                    listener->notifyPeerDisconnected(peer[i], i);
                                deleteClient(i);
                            }
                            else
                            {
                                sem.recvSockId = i;
                                sem.recvSock = peer[i];
                                if(listener)
                                    listener->recvData(peer[i], recvData());
                            }
                        }
                    }
                }
            }
        }
    }
    void Seeder::end() noexcept
    {
        if(mtu != newmtu)
        {
            mtu = newmtu;
            if(buffer) delete[] buffer;
            buffer = new char[mtu+1];
        }
        clearBuffer(0);
    }
    bool Seeder::isConnected() const noexcept
    {
        return IsConnected;
    }
    int  Seeder::shutdown() noexcept
    {
        IsConnected = false;
        //shutdown(sock, 1);
        peer.clear();
        return closesocket(sock)+1;
    }
    void Seeder::clearBuffer(const char c) noexcept
    {
        memset(buffer,c,mtu);
    }
    void Seeder::deleteClient(const unsigned int ss) noexcept
    {
        if(ss > peer.size()) return;
//        shutdown(peer[ss]->sock, 2);
        if(closesocket(peer[ss]->sock) == -1)
            std::cout << "error when closing client socket" << std::endl;
        delete peer[ss];
        peer[ss] = 0;
        peer.erase(peer.begin()+ss);
    }
    void Seeder::deleteClient(option::Client *cl) noexcept
    {
        deleteClient(findClient(cl));
    }
    option::Client* Seeder::getClient(const unsigned int i) const noexcept
    {
        return peer[i];
    }
    unsigned int Seeder::findClient(const option::Client *cl) const noexcept
    {
        for(unsigned int i = 0; i<peer.size(); ++i)
            if(cl->sock == peer[i]->sock) return i;
        return -1;
    }
    void Seeder::exit() noexcept
    {
        if(sem.APP == SAM_SERVEUR) sendMsg("server closed\n");
        while(peer.size()) deleteClient(peer.size());
        peer.clear();
        closesocket( sock );
    }
}
