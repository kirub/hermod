#include <hermod/socket/Address.h>
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <hermod/utilities/Utils.h>


Address::Address()
    : address(0)
    , port(0)
{}

Address::Address(const std::string & AddressAndPort)
    : Address(utils::split(AddressAndPort, ':'))
{
}

Address::Address(std::vector<std::string> AdressAndPort)
    : Address(AdressAndPort[0], atoi(AdressAndPort[1].c_str()))
{
}

Address::Address(const std::string& Address, unsigned short port)
    : Address(utils::split(Address, '.'), port)
{

}

Address::Address(std::vector<std::string> abcd, unsigned short port)
    : Address(atoi(abcd[0].c_str()), atoi(abcd[1].c_str()), atoi(abcd[2].c_str()), atoi(abcd[3].c_str()), port)
{
}

Address::Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port)
    : port(port)
    , address((a << 24) | (b << 16) | (c << 8) | d)
{
}

Address::Address(unsigned int address, unsigned short port)
    : address(address)
    , port(port)
{
}
Address::Address(const sockaddr_in& from)
    : address(ntohl(from.sin_addr.s_addr))
    , port(ntohs(from.sin_port))
{
}

Address::operator std::string() const
{
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(address);
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN);
    return str;
}

Address::operator bool() const
{
    return address != 0;
}

unsigned int Address::GetAddress() const
{
    return address;
}

unsigned short Address::GetPort() const
{
    return port;
}

const sockaddr_in Address::ToSockAddrIn() const
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_port = htons(port);

    return addr;
}


bool Address::operator==(const Address& Rhs) const
{
    return address == Rhs.address;
}
bool Address::operator!=(const Address& Rhs) const
{
    return !operator==(Rhs);
}