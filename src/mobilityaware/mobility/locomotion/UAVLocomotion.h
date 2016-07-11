/*
 * UAVLocomotion.h
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#ifndef UAVLOCOMOTION_H_
#define UAVLOCOMOTION_H_

#include "Locomotion.h"

class UAVLocomotion : public Locomotion
{
public:
    UAVLocomotion();
    virtual ~UAVLocomotion();

    void init(double _speed_kmh, double _updateInterval_ms, ReynoldsMobilityModel *_reynolds);

    SteeringVector handleSteeringVector(SteeringVector _vector);

private:
    double m_speed_mps;
    double m_updateInterval_ms;
};

#endif /* UAVLOCOMOTION_H_ */
