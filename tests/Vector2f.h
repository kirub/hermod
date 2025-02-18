#pragma once

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/replication/NetProperty.h>

enum EnumTest
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
    Vector2f(const Vector2f&);

    Vector2f& operator=(const Vector2f&);

    bool operator==(const Vector2f& Rhs) const
    {
        return X == Rhs.X && Y == Rhs.Y;
    }

    float GetX() const
    {
        return X;
    }

    float GetY() const
    {
        return Y;
    }

private:

    proto::NetProperty<float> X;
    proto::NetProperty<float> Y;
};
