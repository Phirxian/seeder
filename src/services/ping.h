#ifndef PING_H
#define PING_H

#include "SeederListener.h"

namespace network
{
    namespace service
    {
        struct IPHeader
        {
            unsigned char ip_hl:4;
            unsigned char ip_v:4;
            unsigned char ip_tos;
            unsigned short int ip_len;
            unsigned short int ip_id;
            unsigned short int ip_off;
            unsigned char ip_ttl;
            unsigned char ip_p;
            unsigned short int ip_sum;
            unsigned int ip_src;
            unsigned int ip_dst;
        };
        struct ICMPHeader
        {
            unsigned char icmp_type;
            unsigned char icmp_code;
            unsigned short int icmp_cksum;
            union
            {
                struct
                {
                    unsigned short int icmp_id;
                    unsigned short int icmp_seq;
                } echo;
                int	gateway;
                struct
                {
                    unsigned short int __unused;
                    unsigned short int mtu;
                } frag;
            } un;
        };
        //! return -1 on error
        //! if host is unreachable he waiting recipt infinitly
        int ping(const std::string &ip, int timeout_ms = 5000) noexcept;
        unsigned short in_cksum(unsigned short *addr, int len) noexcept;
    }
}

#endif // PING_H
