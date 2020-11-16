#pragma once

#include "SeederListener.h"
#include <vector>

namespace network
{
    class Seeder
    {
        public:
            typedef std::vector<option::Client*> ClientArray;
        public:
            struct SyncNetwork
            {
                fd_set readfs;
                fd_set writefs;
                fd_set exeptfs;
                timeval timeout;
                SOCKADDR_IN sin;
            };
        public:
            Seeder() noexcept;
            virtual ~Seeder() noexcept;

            //! initialise and bind socket
            virtual int init(const option::S_APPLICATION_MODE app, const option::S_TRANSFAIRE_MODE mode,
                             const int port, const int timeout_ms = 300, option::S_PROTOCOL_MODE type = option::SPM_AUTO) noexcept;
            //! clear socket
            virtual void exit() noexcept;

            //! set listener
            virtual void setListener(SeederListener*) noexcept;
            //! get listener
            virtual SeederListener* getListener() const noexcept;

            virtual bool isConnected() const noexcept;
            //! connect to localhost ( 127.0.0.1 )
            virtual int connectLocalHost() noexcept;
            //! connect to
            virtual int connectTo(const std::string &ip) noexcept;
            //! disconnect
            virtual int shutdown() noexcept;

            //! reconnect to current context
            virtual int reConnect(const unsigned int retry = 10) noexcept;
            //! reconnect to new ip
            virtual int reConnectTo(const std::string &ip, const unsigned int retry = 10) noexcept;
            //! allow peer to connect here
            virtual void activeAccept(const bool acpt) noexcept { sem.autoriseAccept = acpt; };

            //! get network socket
            virtual SEEDER_SOCKET& getSocket() noexcept;
            //! get network setting
            virtual Seeder::SyncNetwork* getSyncUtil() noexcept;
            //! get network setting
            virtual option::SocketInfo& getInfo() noexcept;
            //! get connceted client count
            virtual unsigned int getNumberClient() const noexcept;

            //! send data to all peer
            virtual int sendMsg(const std::string &msg, int flags = 0) noexcept;
            //! send data to specified connected peer
            virtual int sendMsgTo(const std::string &msg, const unsigned int n_clt, int flags = 0) noexcept;
            //! send data to specified connected peer
            virtual int sendMsgTo(const std::string &msg, const option::Client *n_clt, int flags = 0) noexcept;
            //! send data to all peer exepted it
            virtual int sendMsgExcepted(const std::string &msg, const unsigned int n_clt, int flags = 0) noexcept;
            //! send data to all peer exepted it
            virtual int sendMsgExcepted(const std::string &msg, const option::Client *n_clt, int flags = 0) noexcept;
            //! send data at unconnected mode
            virtual int sendMsgTo(const std::string &msg, const char *ip, const int port, int flags = 0) noexcept;

            //! send data to all peer
            virtual int sendData(const void *data, const unsigned int lenght, int flags = 0) noexcept;
            //! send data to specified connected peer
            virtual int sendDataTo(const void *data, const unsigned int lenght, const unsigned int n_clt, int flags = 0) noexcept;
            //! send data to specified connected peer
            virtual int sendDataTo(const void *data, const unsigned int lenght, const option::Client *clt, int flags = 0) noexcept;
            //! send data to all peer exepted it
            virtual int sendDataExcepted(const void *data, const unsigned int lenght, const unsigned n_clt, int flags = 0) noexcept;
            //! send data to all peer exepted it
            virtual int sendDataExcepted(const void *data, const unsigned int lenght, const option::Client *clt, int flags = 0) noexcept;
            //! send data at unconnected mode
            virtual int sendDataTo(const void *data, const unsigned int length, const char*ip, const int port, int flags = 0) noexcept;

            //! get received (last) data (with event)
            virtual std::string recvData() const noexcept;
            //! wait data from connected peer
            virtual std::string waitRecvData() noexcept;
            //! wait data from connected peer
            virtual std::string waitRecvDataFrom(const char *ip, const int port) noexcept;

            //! fill buffer with c (0x0 or '\0' is recommended)
            virtual void clearBuffer(const char c = 0x0) noexcept;
            //! disconnect peer with ss = peer array position
            virtual void deleteClient(const unsigned int ss) noexcept;
            //! disconnect peer
            virtual void deleteClient(option::Client *cl) noexcept;
            //! get peer position in array from peer data
            virtual unsigned int findClient(const option::Client *cl) const noexcept;
            //! get peer with peer array position
            virtual option::Client* getClient(const unsigned int) const noexcept;

            //! set buffer size (default: 1492 always > 0, not unsigned for warning remove)
            virtual void setMTU(const int i) noexcept;
            //! get buffer size
            virtual int getMTU() const noexcept;

            //! asynchrone waiting data from all peer
            virtual void run() noexcept;
            //! clear buffer and event
            virtual void end() noexcept;
        protected:
            void setupTimeout(const int ms) noexcept;

            SEEDER_SOCKET sock;
            unsigned int  recsize;
            char         *buffer;
            ClientArray   peer;

            option::SocketInfo  sem;
            SeederListener     *listener;
            struct SyncNetwork *sync;
        private:
            int mtu, newmtu;
            bool IsConnected;
    };
}

