#include "http.h"
#include "SeederClient.h"
#include <iostream>

namespace network
{
    namespace service
    {
        std::string getRemoteFile(const std::string &url, const std::string &file, int timeout) noexcept
        {
            std::string data;
            HttpListener tmp;
            
            SocketInfo info(option::STM_TCP, option::SPT_HTTP, timeout);
            SeederClient *se = new SeederClient(&tmp);
            se->init(info);
            se->connectTo(url);

            if(!se->isConnected())
            {
                delete se;
                return "Connection fails\n";
            }

            std::string request = "GET /";
                request += file;
                request += " HTTP/1.0\r\nHost: ";
                request += url;
                request += "\r\nUser-Agent: HTMLGET 1.0\r\n\r\n";
                
            SendingMessage msg(request.c_str(), request.size());
            se->sendData(msg);

            while(se->isConnected() && !tmp.breakLoop)
                se->run();

            delete se;
            return tmp.data;
        }

        std::string removeHttpHeaderInformation(const std::string &data) noexcept
        {
            int size = data.size();
            long content_length = data.find("Content-Length: ", 0);
            if(content_length)
            {
                long endLine = data.find("\n", content_length);
                size = std::stoi(data.substr(content_length+16, endLine-content_length-16), nullptr);
                return data.substr(data.size()-size, data.size());
            }
            return data;
        }
        
        // *******************************************************
        
        void HttpListener::notifySendError(const SendingMessage &msg, int error)
        {
            std::cout << msg.to_string() << std::endl;
            breakLoop |= (msg.length == 0);
        }

        void HttpListener::notifyPeerLost(ConnectedClient*, int index)
        {
            breakLoop = true;
        }

        void HttpListener::notifyPeerCrashed(ConnectedClient*, int index)
        {
            breakLoop = true;
        }

        void HttpListener::notifyPeerConnected(ConnectedClient*, int index)
        {
            breakLoop = true;
        }

        void HttpListener::notifyPeerDisconnected(ConnectedClient*, int index)
        {
            breakLoop = true;
        }

        void HttpListener::notifyConnected(bool i)
        {
            breakLoop = !i;
        }

        void HttpListener::notifyConnectionLost()
        {
            breakLoop = true;
        }
        
        void HttpListener::recvData(const ReceivedMessage &msg)
        {
            data += msg.to_string();
        }
    }
}
