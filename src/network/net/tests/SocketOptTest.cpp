#include "network/base/SocketOpt.h"
#include <iostream>



using namespace vdse::network;

void TestClient()
{
    int sock = SocketOpt::CreatNonblockingTcpSocket(AF_INET);
    if (sock < 0)
    {
        std::cerr << "socket failed. sock " << sock << " errno : " << errno << std::endl;
        return;
    }

    InetAddress server("192.168.159.131:34444");
    SocketOpt opt(sock);
    opt.SetNonBlocking(false);
    auto ret = opt.Connect(server);
    std::cout << "connect ret : " << ret << " errno : " << errno << std::endl
                << " local : " << opt.GetLocalAddr()->ToIpPort() << std::endl
                << " peer : " << opt.GetPeerAddr() -> ToIpPort() << std::endl
                << std::endl;
    
}


int main(int argc, char const *argv[])
{
    TestClient();
    return 0;
}
