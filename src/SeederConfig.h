#pragma once

#include "NetworkAPI.h"
#include <string>

namespace network
{
    class ConnectedClient;
    
    namespace option
    {   
        enum S_TRANSFAIRE_MODE
        {
            STM_TCP = SOCK_STREAM,           /* Sequenced, reliable, connection-based byte streams. */
            STM_UDP = SOCK_DGRAM,            /* Connectionless, unreliable datagrams of fixed maximum length. */
            STM_RAW = SOCK_RAW,              /* Raw protocol interface. */
        };
        
        enum S_PROTOCOL_MODE
        {
            SPM_AUTO = -1,                   /* automatic detecte protocol from S_TRANSFAIRE_MODE */
            SPM_IP = IPPROTO_IP,	           /* Dummy protocol for TCP.  */
            SPM_HOPOPTS = IPPROTO_HOPOPTS,   /* IPv6 Hop-by-Hop options.  */
            SPM_ICMP = IPPROTO_ICMP,	       /* gamet Control Message Protocol.  */
            SPM_IGMP = IPPROTO_IGMP,	       /* gamet Group Management Protocol. */
            SPM_TCP = IPPROTO_TCP,	         /* Transmission Control Protocol.  */
            SPM_PUP = IPPROTO_PUP,	         /* PUP protocol.  */
            SPM_UDP = IPPROTO_UDP,	         /* User Datagram Protocol.  */
            SPM_IDP = IPPROTO_IDP,	         /* XNS IDP protocol.  */
            SPM_IPV6 = IPPROTO_IPV6,         /* IPv6 header.  */
            SPM_ROUTING = IPPROTO_ROUTING,   /* IPv6 routing header.  */
            SPM_FRAGMENT = IPPROTO_FRAGMENT, /* IPv6 fragmentation header.  */
            SPM_ESP = IPPROTO_ESP,           /* encapsulating security payload.  */
            SPM_AH = IPPROTO_AH,             /* authentication header.  */
            SPM_ICMPV6 = IPPROTO_ICMPV6,     /* ICMPv6.  */
            SPM_NONE = IPPROTO_NONE,         /* IPv6 no next header.  */
            SPM_DSTOPTS = IPPROTO_DSTOPTS,   /* IPv6 destination options.  */
            SPM_RAW = IPPROTO_RAW,	         /* Raw IP packets.  */
            #ifndef __WIN32
                SPM_IPIP = IPPROTO_IPIP,	       /* IPIP tunnels (older KA9Q tunnels use 94).  */
                SPM_EGP = IPPROTO_EGP,	         /* Exterior Gateway Protocol.  */
                SPM_TP = IPPROTO_TP,	           /* SO Transport Protocol Class 4.  */
                SPM_DCCP = IPPROTO_DCCP,	       /* Datagram Congestion Control Protocol.  */
                SPM_RSVP = IPPROTO_RSVP,	       /* Reservation Protocol.  */
                SPM_UDPLITE = IPPROTO_UDPLITE,   /* UDP-Lite protocol.  */
                SPM_GRE = IPPROTO_GRE,	         /* General Routing Encapsulation.  */
                SPM_MTP = IPPROTO_MTP,	         /* Multicast Transport Protocol.  */
                SPM_ENCAP = IPPROTO_ENCAP,	     /* Encapsulation Header.  */
                SPM_PIM = IPPROTO_PIM,	         /* Protocol Independent Multicast.  */
                SPM_COMP = IPPROTO_COMP,	       /* Compression Header Protocol.  */
                SPM_SCTP = IPPROTO_SCTP,	       /* Stream Control Transmission Protocol.  */
            #endif
        };
        
        enum S_PORT_TYPE
        {
            /** list of special service/application port used */
            /************ SERVICE ***********/
            SPT_ECHO = 7,		      /* Echo service.  */
            SPT_DISCARD = 9,	    /* Discard transmissions service.  */
            SPT_SYSTAT = 11,      /* System status service.  */
            SPT_DAYTIME = 13,	    /* Time of day service.  */
            SPT_NETSTAT = 15,	    /* Network status service. */
            SPT_TIMESERVER = 37,  /* Timeserver service.  */
            SPT_NAMESERVER = 42,  /* Domain Name Service.  */
            SPT_MTP = 57,
            SPT_SUPDUP = 95,	    /* SUPDUP protocol.  */
            SPT_EXECSERVER = 512,	/* execd service.  */
            SPT_LOGINSERVER = 513,/* rlogind service.  */
            SPT_CMDSERVER = 514,
            SPT_EFSSERVER = 520,
            SPT_BIFFUDP = 512,
            SPT_WHOSERVER = 513,
            SPT_ROUTESERVER = 520,
            /************ SERVEUR ***********/
            SPT_FTP = 21,         /* File Transfer Protocol.  */
            SPT_TELNET = 23,      /* Telnet protocol.  */
            SPT_SMTP = 25,        /* Simple Mail Transfer Protocol.  */
            SPT_DNS = 53,
            SPT_WHOIS = 63,       /* gamet Whois service.  */
            SPT_TFTP = 69,        /* Trivial File Transfer Protocol.  */
            SPT_GOPHER = 70,
            SPT_RJE = 77,
            SPT_FINGER = 79,      /* Finger service.  */
            SPT_NNTP = 119,
            SPT_LOTUS = 1352,
            SPT_RPS = 799,
            SPT_HTTP = 80,
            SPT_HTTP2 = 443,
            SPT_POP2 = 109,
            SPT_POP3 = 110,
            SPT_LDAP = 389,
            SPT_LDAP2 = 636,
            SPT_IRC = 6667,
            SPT_IRC2 = 194,
            SPT_IRC3 = 5100,
            SPT_IMAP = 143,
            SPT_IMAP2 = 585,
            SPT_IMAP3 = 993,
            /********** PEER TO PEER ********/
            SPT_EMULE = 4662,
            SPT_EMULE2 = 4771,
            SPT_EMULE3 = 4672,
            SPT_CHAREAZA = 6346,
            SPT_LIMEWIRE = 6346,
            SPT_EXEEM = 6881,
            SPT_PANDO = 6881,
            SPT_ASUREUS = 6881,
            SPT_BITCOMET = 6881,
            SPT_BITTORRENT = 6881,
            SPT_TORRENTTOPIA = 6881,
            SPT_MICROTORRENT = 6881,
            SPT_EASYTORRENT = 6881,
            /************* APP **************/
            SPT_RADMIN = 4899,
            SPT_LAPLINK = 1547,
            SPT_NETBIOS = 137,
            SPT_VNC = 5500,
            SPT_VNC2 = 5800,
            SPT_VNC3 = 5900,
            SPT_WIN2K = 3389,
            SPT_PCANYWHERE_TCP = 5631,
            SPT_PCANYWHERE_TCP2 = 65301,
            SPT_PCANYWHERE_UDP = 22,
            SPT_PCANYWHERE_UDP2 = 5632,
            SPT_REMOTEANYTHING = 3996,
            SPT_REMOTEANYTHING2 = 3999,
            SPT_CARBONCOPY_TCP = 1680,
            SPT_CARBONCOPY_UDP = 1023,
            SPT_GNUTELLA_TCP = 1214,
            SPT_GNUTELLA_UDP = 6346,
            /********* AUDIO/VIDEO **********/
            SPT_QUICKTIME = 6970,
            SPT_NET2PHONE = 6613,
            SPT_NETMEETING_UDP = 1024,
            SPT_NETMEETING_TCP = 1503,
            SPT_NETMEETING_TCP1 = 1720,
            SPT_NETMEETING_TCP2 = 1731,
            SPT_NETMEETING_TCP3 = 522,
            SPT_NETMEETING_TCP4 = 389,
            /************* VPN **************/
            SPT_PPTP = 1723,
            SPT_IPSEC_L2TP = 500,
            SPT_IPSEC_L2TP2 = 4500,
        };
    }
}

