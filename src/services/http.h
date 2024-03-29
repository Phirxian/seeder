#ifndef HTTP_H
#define HTTP_H

#include "SeederListener.h"

namespace network
{
    class ConnectedClient;
    
    namespace service
    {
        inline void replace(std::string &source, const std::string &last, const std::string &by) noexcept
        {
            std::string tnew;
            long pos = 0, get = source.find(last,0);
            while(get <= source.size())
            {
                get = source.find(last,pos);
                if(get < 0) break;
                else
                {
                    tnew += source.substr(pos, get-pos);
                    tnew += by;
                    pos = get+last.size();
                }
            }
            source = tnew;
        }
        
        struct htmlParsing
        {
            std::string rss;
            std::string getRssNewsTitle() const noexcept
            {
                long titleBegin = rss.find("<title><![CDATA[");
                long titleEnd = rss.find("]]></title>");
                if(titleBegin && titleEnd)
                    return rss.substr(titleBegin+16, titleEnd-titleBegin-16);
                return "News";
            }
            std::string getRssNewsContent() const noexcept
            {
                long contentBegin = rss.find("<description><![CDATA[");
                long contentEnd = rss.find("]]></description>");
                if(contentBegin >= 0 && contentEnd >= 0)
                    return rss.substr(contentBegin+22, contentEnd-contentBegin-22);
                return "\0";
            }
            std::string parse(std::string text) const noexcept
            {
                std::string a = text;
                replace(a, "<br />", "\n");
                replace(a, "<br/>", "\n");
                return a;
            }
        };
        
        class HttpListener : public SeederListener
        {
            public:
                HttpListener() : breakLoop(false) { }

                virtual void recvData(const ReceivedMessage &msg);

                virtual void notifySendError(const SendingMessage &msg, int error);
                virtual void notifySendError(const UnconnectedMessage&, int error) {}

                virtual void notifyPeerLost(ConnectedClient*, int index);
                virtual void notifyPeerCrashed(ConnectedClient*, int index);
                virtual void notifyPeerConnected(ConnectedClient*, int index);
                virtual void notifyPeerDisconnected(ConnectedClient*, int index);

                virtual void notifyConnected(bool);
                virtual void notifyConnectionLost();

                bool breakLoop;
                std::string data;
            protected:
            private:
        };
        
        std::string getRemoteFile(const std::string &url, const std::string &file, int timeout = 240) noexcept;
        std::string removeHttpHeaderInformation(const std::string&) noexcept;
    }
}

#endif // HTTP_H
