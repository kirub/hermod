#pragma once

#include <hermod/platform/Platform.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

class HERMOD_API Address
{
public:

    Address();

    Address(const std::string& AddressAndPort);
    Address(std::vector<std::string> AdressAndPort);
    Address(const std::string& Address, unsigned short port);
    Address(std::vector<std::string> abcd, unsigned short port);
    Address(unsigned char a,
        unsigned char b,
        unsigned char c,
        unsigned char d,
        unsigned short port);

    Address(unsigned int address,
        unsigned short port);
    Address(const sockaddr_in& from);

    operator std::string() const;
    operator bool() const;
     
    const sockaddr_in ToSockAddrIn() const;

    unsigned int GetAddress() const;
    unsigned short GetPort() const;

    bool operator==(const Address& Rhs) const;
    bool operator!=(const Address& Rhs) const;

private:

    unsigned int address;
    unsigned short port;
};