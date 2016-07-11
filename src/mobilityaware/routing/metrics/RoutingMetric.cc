/*
 * RoutingMetric.cpp
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#include "RoutingMetric.h"

RoutingMetric::RoutingMetric()
{


}

RoutingMetric::~RoutingMetric()
{
}

double RoutingMetric::scale(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float RoutingMetric::scaleValue(float _value, float _inMin, float _inMax, float _outMin, float _outMax)
{
    return (_value-_inMin) * (_outMax-_outMin) / (_inMax-_inMin) + _outMin;
}

float RoutingMetric::limitValue(float _value, float _min, float _max)
{
    float value = _value;
    if(value<_min)
        value = _min;
    else if(value>_max)
        value = _max;

    return value;
}

float RoutingMetric::limitAbsoluteValue(float _value, float _max)
{
    float value = _value;
    if(value<-_max)
        value = -_max;
    else if(value>_max)
        value = _max;

    return value;
}
