#include "NetFixture.h"

NetFixture::NetFixture()
{
    ISocket::Initialize();
}

NetFixture::~NetFixture()
{
    ISocket::Shutdown();
}