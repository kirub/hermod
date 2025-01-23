#include "Vector2f.h"

Vector2f::Vector2f()
    : Vector2f(0, 0)
{
}

Vector2f::Vector2f(float InX, float InY)
    : X(InX)
    , Y(InY)
{
    AddProperty(X);
    AddProperty(Y);
}