#pragma once

#include <gtest/gtest.h>

#include <hermod/socket/NetworkHandler.h>
#include "../Testables/ConnectionTestableLoopback.h"

class NetFixture
{
protected:

    NetFixture();
    virtual ~NetFixture();

};