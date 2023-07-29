#include "NetworkAPI.h"
#include "ping.h"
#include <ctime>

#ifndef __linux
#   include <winsock2.h>
#else
#   include <fcntl.h>
#   include <netdb.h>
#   include <sys/socket.h>
#   include <sys/ioctl.h>
#   include <linux/ip.h>
#   include <linux/icmp.h>
#endif

#include "SeederClient.h"

namespace network
{
    namespace service
    {
        unsigned short in_cksum(unsigned short *buf, int len) noexcept
        {
            register long sum = 0;
            unsigned short answer = 0;

            while(len > 1)
            {
                sum += *buf++;
                len -= 2;
            }
            if(len == 1)
            {
                *(unsigned char *)(&answer) = *(unsigned char *)buf;
                sum += answer;
            }

            sum = (sum>>16) + (sum &0xffff);
            sum += (sum>>16);
            answer = ~sum;
            return answer;
        }
        
        int ping(const std::string &ip, int timeout) noexcept
        {
            std::string request;
            request.reserve(sizeof(IPHeader)+sizeof(ICMPHeader));

            IPHeader *ipheader = (IPHeader*)(request.c_str());
            ICMPHeader *icmp = (ICMPHeader*)(request.c_str()+sizeof(IPHeader));

            ipheader->ip_hl = 5;
            ipheader->ip_v  = 4;
            ipheader->ip_tos = 0;
            ipheader->ip_len = sizeof(IPHeader)+sizeof(ICMPHeader);
            ipheader->ip_id = 0;
            ipheader->ip_off = 0;
            ipheader->ip_ttl = 64;
            ipheader->ip_p = IPPROTO_ICMP;
            ipheader->ip_src = inet_addr("0.0.0.0");
            ipheader->ip_dst = inet_addr(inet_ntoa(*(in_addr*)*(gethostbyname(ip.c_str())->h_addr_list)));
            ipheader->ip_sum = in_cksum((unsigned short*)ipheader,sizeof(IPHeader));

            icmp->icmp_type = ICMP_ECHO;
            icmp->icmp_code = 0;
            icmp->icmp_cksum = 0;
            icmp->un.echo.icmp_id = getpid() & 0xFFFF;
            icmp->un.echo.icmp_seq = htons(1);
            icmp->icmp_cksum = in_cksum((unsigned short*)ipheader,sizeof(IPHeader)+sizeof(ICMPHeader));
            
            SocketInfo info(option::STM_RAW, 0, timeout, option::SPM_ICMP);
            SeederClient *se = new SeederClient();
            se->init(info);
            se->setMTU(request.size());

            int hold = 1;
            setsockopt(se->getSocket(), SOL_SOCKET, SO_DEBUG, (char *)&hold, sizeof(hold));
            setsockopt(se->getSocket(), IPPROTO_IP, IP_HDRINCL, &hold, sizeof(hold));
            setsockopt(se->getSocket(), SOL_SOCKET, SO_BROADCAST, (char *)&hold, sizeof(hold));

            UnconnectedMessage msg(request);
            msg.ip = ip.c_str();
            msg.port = 0;
            se->sendDataTo(msg);
            
            int start = clock();
            msg = se->waitRecvDataFrom(ip.c_str(), 0);
            request = msg.to_string();
            int end = clock();

            ipheader = (IPHeader*)(request.c_str());
            icmp = (ICMPHeader*)(request.c_str()+sizeof(IPHeader));

            switch(icmp->icmp_type)
            {
                case 3:
                    printf("Destination host unreachable");
                break;
                case 4:
                    printf("Source Quench");
                break;
                case 11:
                    printf("Time Exceeded");
                break;
                case 12:
                    printf("Bad IP header");
                break;
            };

            delete se;
            return end-start;
        }
    }
}
