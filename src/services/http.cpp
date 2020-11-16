#include "http.h"
#include "Seeder.h"
#include <iostream>

namespace network
{
    namespace service
    {
        std::string getRemoteFile(const std::string &url, const std::string &file, int timeout) noexcept
        {
            std::string data;
            Seeder *se = new Seeder();

            HttpListener tmp;
            se->setListener(&tmp);

            se->init(option::SAM_CLIENT, option::STM_TCP, option::SPT_HTTP, timeout);
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
            se->sendMsg(request);

            while(se->isConnected() && !tmp.breakLoop)
            {
                se->run();
                se->end();
            }

            delete se;
            return tmp.data;
        }
        std::string removeHttpHeaderInformation(const std::string &data) noexcept
        {
            int size = data.size();
            long content_lenght = data.find("Content-Length: ", 0);
            if(content_lenght)
            {
                long endLine = data.find("\n", content_lenght);
                size = std::stoi(data.substr(content_lenght+16, endLine-content_lenght-16), nullptr);
                return data.substr(data.size()-size, data.size());
            }
            return data;
        }
        
        // *******************************************************
        
        void HttpListener::notifySendError(const std::string &msg, int error, int flags, const network::option::Client *c)
        {
            std::cout << msg << std::endl;
            breakLoop |= !msg.empty();
        }
        void HttpListener::notifyPeerLost(network::option::Client*, int index)
        {
            breakLoop = true;
        }
        void HttpListener::notifyPeerCrashed(network::option::Client*, int index)
        {
            breakLoop = true;
        }
        void HttpListener::notifyPeerConnected(network::option::Client*, int index)
        {
            breakLoop = true;
        }
        void HttpListener::notifyPeerDisconnected(network::option::Client*, int index)
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
        void HttpListener::recvData(const std::string &msg)
        {
            data += msg;
        }
        void HttpListener::recvData(network::option::Client *c, const std::string &msg)
        {
            data += msg;
        }
    }
}
