#include"Server.h"
#include<thread>
#include"AddressManager.h"

int main(int argc, char* argv[])
{
    if(argc ==2)
    {
    using namespace boost::asio;
    AddrManager addrMgr;
    //addrMgr.addIp("10.10.1.171");
    addrMgr.addIp("127.0.0.1");
    Server s(atoi(argv[1]), addrMgr);
    s.start();
    s.runMultiThread();
    //run the server on multiple threads
    return 0;
    }
}
