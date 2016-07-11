/*
 * SteeringVector.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#include <SteeringVector.h>
#include <iostream>
#include <math.h>
SteeringVector::SteeringVector() :
    isValid(false)
{


}

SteeringVector::SteeringVector(double _x, double _y, double _z) :
    x(_x),
    y(_y),
    z(_z),
    isValid(true)
{

}

SteeringVector::~SteeringVector()
{

}

SteeringVector SteeringVector::operator+(const SteeringVector &_rhs)
{
    return SteeringVector(x + _rhs.x, y + _rhs.y, z + _rhs.z);
}

SteeringVector SteeringVector::operator+=(const SteeringVector &_rhs)
{
    x += _rhs.x;
    y += _rhs.y;
    z += _rhs.z;

    return *this;
}

SteeringVector SteeringVector::operator-(const SteeringVector &_rhs)
{
    return SteeringVector(x - _rhs.x, y - _rhs.y, z - _rhs.z);
}

SteeringVector SteeringVector::operator-=(const SteeringVector &_rhs)
{
    x -= _rhs.x;
    y -= _rhs.y;
    z -= _rhs.z;

    return *this;
}

SteeringVector SteeringVector::operator*(const double &_rhs)
{
    return SteeringVector(x * _rhs, y * _rhs, z * _rhs);
}

SteeringVector SteeringVector::operator*=(const double &_rhs)
{
    x *= _rhs;
    y *= _rhs;
    z *= _rhs;

    return *this;
}

SteeringVector SteeringVector::operator/(const double &_rhs)
{
    return SteeringVector(x / _rhs, y / _rhs, z / _rhs);
}

 SteeringVector SteeringVector::operator/=(const double &_rhs)
{
    x /= _rhs;
    y /= _rhs;
    z /= _rhs;

    return *this;
}

double SteeringVector::norm()
{
    return sqrt(x*x+y*y+z*z);
}

void SteeringVector::output()
{
    std::cout << "(" << x << "," << y << "," << z << ")" << std::endl;
}

void SteeringVector::setValues(double _x, double _y, double _z)
{
    x = _x;
    y = _y;
    z = _z;

    isValid = true;
}

