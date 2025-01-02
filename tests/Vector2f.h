#pragma once

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/replication/NetProperty.h>

enum Test
{
    A, B, C,
    Count
};

class Vector2f
    : public proto::INetObject
{

    CLASS_ID(Vector2f)

public:

    Vector2f();

    Vector2f(float InX, float InY);

private:

    proto::NetProperty<float> X;
    proto::NetProperty<float> Y;
};
