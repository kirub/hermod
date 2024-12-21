#include "Vector2f.h"

Vector2f::Vector2f()
    : X(*this, 0)
    , Y(*this, 0)
{
}

Vector2f::Vector2f(float InX, float InY)
    : X(*this, InX)
    , Y(*this, InY)
{
}