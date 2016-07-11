/*
 * RoutingMetric.h
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#ifndef ROUTINGMETRIC_H_
#define ROUTINGMETRIC_H_

#include "csimplemodule.h"

class RoutingMetric : public cSimpleModule
{
public:
    RoutingMetric();
    virtual ~RoutingMetric();

    static double scale(double x, double in_min, double in_max, double out_min, double out_max);

    static float scaleValue(float _value, float _inMin, float _inMax, float _outMin, float _outMax);
    static float limitValue(float _value, float _min, float _max);
    static float limitAbsoluteValue(float _value, float _max);
};

#endif /* ROUTINGMETRIC_H_ */
