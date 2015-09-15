#ifndef ADDRESSMANAGER_H
#define ADDRESSMANAGER_H

#include<string>
#include<set>
#include<boost/asio/ip/tcp.hpp>


class AddrManager
{
private:
    std::set<std::string> m_ipSet;
public:
    AddrManager();
    //delete copy operations
    AddrManager(const AddrManager& source)=delete;
    AddrManager& operator=(const AddrManager& source)=delete;
    //default move operations
    AddrManager(AddrManager&& source)=default;
    AddrManager& operator=(AddrManager&& source)=default;
    //operations on ip addresses
    void addIp(std::string argIp);
    bool findIp(std::string argIp);
    bool removeIp(std::string argIp);
    //util functions
    std::string static getIp(boost::asio::ip::tcp::acceptor& argAcceptor);
    std::string static getIp(boost::asio::ip::tcp::socket& argSocket);

};

#endif // ADDRESSMANAGER_H
