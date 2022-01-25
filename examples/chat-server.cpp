#include "SeederServer.h"
#include <iostream>

using namespace network;

class ChatServer : public SeederListener
{
    public:
        ChatServer()
        {
        }
        
        void setSeederSocket(SeederServer *socket)
        {
            se = socket;
        }

        virtual void recvData(const ReceivedMessage &msg)
        {
            std::string message;
            message += std::to_string(msg.recvSock->sock) + " >> ";
            message += std::string(msg.buffer, msg.length);
            std::cout << message << std::endl;
            
            SendingMessage send(message.c_str(), message.size());
            se->sendData(send);
            //se->sendDataExcepted(send, msg.recvSock);
        }

        virtual void notifySendError(const SendingMessage &msg, int error)
        {
        }
        
        virtual void notifySendError(const UnconnectedMessage&, int error)
        {
        }
        
        virtual void notifyPeerLost(ConnectedClient *client, int index)
        {
            notifyPeerDisconnected(client, index);
        }
        
        virtual void notifyPeerCrashed(ConnectedClient *client, int index)
        {
            notifyPeerDisconnected(client, index);
        }
        
        virtual void notifyPeerConnected(ConnectedClient *client, int index)
        {
            std::string message = "new client appear >> ";
            message += std::to_string(client->sock);
            
            std::cout << message << std::endl;
            SendingMessage send(message.c_str(), message.size()+1);
            se->sendData(send);
        }
        
        virtual void notifyPeerDisconnected(ConnectedClient *client, int index)
        {
            std::string message = "a client has leaved >> ";
            message += std::to_string(client->sock);
            
            std::cout << message << std::endl;
            
            SendingMessage send(message.c_str(), message.size()+1);
            se->sendData(send);
        }

        virtual void notifyConnected(bool) {}
        virtual void notifyConnectionLost() {}
    
    protected:
        SeederServer *se;
};

int main(int argc, const char **argv)
{
    ChatServer *srv = new ChatServer();
    SeederServer *se = new SeederServer(srv);
    srv->setSeederSocket(se);
    
    SocketInfo info(option::STM_TCP, 4096, 300);
    se->init(info);
    se->activeAccept(true);
    
    while(se->isConnected())
        se->run();
        
    delete se;
}
