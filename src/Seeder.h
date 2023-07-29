#pragma once

#include "SeederListener.h"
#include <vector>

namespace network
{
    struct SocketInfo
    {
        SocketInfo() : port(0), timeout(300)
        {
        }
        
        SocketInfo(const option::S_TRANSFAIRE_MODE mode, const int port, const int timeout_ms = 300, option::S_PROTOCOL_MODE type = option::SPM_AUTO) noexcept
        {
            this->mode = mode;
            this->port = port;
            this->timeout = timeout_ms;
            
            if(type == option::SPM_AUTO)
            {
                switch(mode)
                {
                    case option::STM_TCP: this->type = option::SPM_TCP; break;
                    case option::STM_UDP: this->type = option::SPM_UDP; break;
                    case option::STM_RAW: this->type = option::SPM_RAW; break;
                    default:      this->type = option::SPM_TCP; break;
                }
            }
        }
        
        //! socket mode (DGRAM/RAW/STREAM)
        option::S_TRANSFAIRE_MODE mode;
        //! type of the socket TCP/UDP/RAW
        option::S_PROTOCOL_MODE type;
        //! used port (local for server and remote for client)
        int port;
        //! recv/send/accept/... timeout
        int timeout;
    };
    
    class Seeder
    {
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
            Seeder(SeederListener *listener = nullptr) noexcept;
            virtual ~Seeder() noexcept;

            //! initialise and bind socket
            virtual int init(const SocketInfo&) noexcept = 0;

            //! set listener
            virtual void setListener(SeederListener *l) noexcept { listener = l; }
            //! get listener
            virtual SeederListener* getListener() const noexcept { return listener; }
            
            //! disconnect
            virtual int shutdown() noexcept;

            //! get network socket
            virtual SEEDER_SOCKET& getSocket() noexcept;
            //! get network setting
            virtual Seeder::SyncNetwork* getSyncUtil() noexcept;
            //! get network setting
            virtual SocketInfo& getInfo() noexcept;
            //! check if the connection is open
            virtual bool isConnected() const noexcept { return IsConnected; }
            
            //! wait data from the socket
            virtual ReceivedMessage waitRecvData() noexcept;
            
            //! send data in UDP mode
            virtual int sendDataTo(const UnconnectedMessage&) noexcept;
            //! wait data in UDP mode
            virtual UnconnectedMessage waitRecvDataFrom(const char *ip, const int port) noexcept;

            //! set buffer size (default: 1492 always > 0, not unsigned for warning remove)
            void setMTU(const int i) noexcept { mtu = i; }
            //! get buffer size
            int getMTU() const noexcept { return mtu; }
            
            //! asynchrone waiting data from all peer
            virtual void run() noexcept = 0;
            
        protected:
            void setupTimeout(const int ms) noexcept;
            
            SEEDER_SOCKET sock;
            unsigned int  recv_size;
            unsigned int  mtu;
            bool          IsConnected;

            SocketInfo          sem;
            SeederListener     *listener;
            struct SyncNetwork *sync;
    };
}

