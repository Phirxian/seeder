#pragma once

extern "C"
{
    #if defined __WIN32 || defined WIN64
        #include <winsock2.h>

        #define ioctl(a, b, c)		ioctlsocket(a, b, (unsigned long*)(c))

        #undef EAGAIN
        #define EAGAIN WSAEWOULDBLOCK

        #undef ECONNABORTED
        #define ECONNABORTED WSAECONNABORTED

        #undef ENOTCONN
        #define ENOTCONN WSAENOTCONN

        #undef ECONNREFUSED
        #define ECONNREFUSED WSAECONNREFUSED

        #undef EBADF
        #define EBADF WSAENOTSOCK

        #undef ETIMEDOUT
        #define ETIMEDOUT WSAETIMEDOUT
        
        #define WINDOWS

        typedef unsigned int   u_int;
        typedef unsigned char  u_char;
        typedef unsigned short u_short;
        typedef unsigned long  u_long;
        #define		MSG_NOSIGNAL	0
    #elif defined __linux
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <unistd.h>
        
        #define INVALID_SOCKET -1
        #define SOCKET_ERROR -1
        #define closesocket(s) close (s)
        #define LINUX

        typedef void* sockopt;
        typedef struct sockaddr_in SOCKADDR_IN;
        typedef struct sockaddr    SOCKADDR;
        typedef struct sockaddr_in SOCKADDR_IN;
        typedef struct sockaddr    SOCKADDR;
    #else
        typedef char* sockopt;
        typedef struct sockaddr_in SOCKADDR_IN;
        typedef struct sockaddr    SOCKADDR;
        typedef struct sockaddr_in SOCKADDR_IN;
        typedef struct sockaddr    SOCKADDR;
    #endif
    
    typedef unsigned int SEEDER_SOCKET;
}
