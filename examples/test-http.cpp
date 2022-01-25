#include "services/http.h"
#include <iostream>

using namespace network;

int main(int argc, const char **argv)
{
    std::string data = service::getRemoteFile("ftp.osuosl.org", "/pub/slackware/slackware64-current/ChangeLog.txt");
    std::cout << data << std::endl;
}
