#include"AddressManager.h"
#include<boost/asio/ip/address.hpp>
using std::string;
using std::set;

AddrManager::AddrManager()
   :m_ipSet()
{}

void AddrManager::addIp(std::string argIp)
{
   m_ipSet.insert(argIp);
}

bool AddrManager::findIp(string argIp)
{
    set<string>::iterator it = m_ipSet.find(argIp);
    if( it != m_ipSet.end() )
        return true;
    else
        return false;
}

bool AddrManager::removeIp(std::string argIp)
{
    set<string>::iterator it = m_ipSet.find(argIp);

    if( it == m_ipSet.end() )
        return false;

    m_ipSet.erase(it);
    return true;

}
std::string AddrManager::getIp(boost::asio::ip::tcp::acceptor& argAcceptor)
{
    using boost::asio::ip::tcp;
    using namespace boost::asio::ip;

    //get endpoint of acceptor
    tcp::endpoint acceptorEndPoint = argAcceptor.local_endpoint();

    //get the ip addr
    address addr=acceptorEndPoint.address();

    //convert the addr to string
    std::string stringAddr = addr.to_string();

    return stringAddr;
}
std::string AddrManager::getIp(boost::asio::ip::tcp::socket& argSocket)
{
    using boost::asio::ip::tcp;
    using namespace boost::asio::ip;

    //get the endpoint
    tcp::endpoint socketEndPoint = argSocket.remote_endpoint();

    //get the ip addr
    address addr=socketEndPoint.address();

    //convert the addr to string
    std::string stringAddr = addr.to_string();

    return stringAddr;
}
