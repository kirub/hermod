#include "Vector2f.h"

Vector2f::Vector2f()
    : Vector2f(0, 0)
{
}

Vector2f::Vector2f(float InX, float InY)
    : X(PROPERTY(InX))
    , Y(PROPERTY(InY))
{
    AddProperty(X);
    AddProperty(Y);
}

Vector2f::Vector2f(const Vector2f& Rhs)
    : Vector2f(Rhs.X,Rhs.Y)
{

}


Vector2f& Vector2f::operator=(const Vector2f& Rhs)
{
    X = Rhs.GetX();
    Y = Rhs.GetY();

    return *this;
}