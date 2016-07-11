/*
 * SteeringVector.h
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#ifndef STEERINGVECTOR_H_
#define STEERINGVECTOR_H_

class SteeringVector
{
public:
    SteeringVector();
    SteeringVector(double _x, double _y, double _z);
    virtual ~SteeringVector();

    void setValues(double _x, double _y, double _z);

    SteeringVector operator+(const SteeringVector &_rhs);
    SteeringVector operator+=(const SteeringVector &_rhs);
    SteeringVector operator-(const SteeringVector &_rhs);
    SteeringVector operator-=(const SteeringVector &_rhs);

    SteeringVector operator*(const double &_rhs);
    SteeringVector operator*=(const double &_rhs);
    SteeringVector operator/(const double &_rhs);
    SteeringVector operator/=(const double &_rhs);


    double norm();

    void output();

public:
    double x;
    double y;
    double z;

    bool isValid;
};

#endif /* STEERINGVECTOR_H_ */
