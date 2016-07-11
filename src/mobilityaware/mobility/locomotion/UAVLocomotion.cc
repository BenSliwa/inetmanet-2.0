/*
 * UAVLocomotion.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#include <UAVLocomotion.h>

Define_Module(UAVLocomotion);

UAVLocomotion::UAVLocomotion() : Locomotion()
{

}

UAVLocomotion::~UAVLocomotion()
{

}

void UAVLocomotion::init(double _speed_kmh, double _updateInterval_ms, ReynoldsMobilityModel *_reynolds)
{
    Locomotion::init(_reynolds);
    m_speed_mps = _speed_kmh/3.6;
    m_updateInterval_ms = _updateInterval_ms;
}

SteeringVector UAVLocomotion::handleSteeringVector(SteeringVector _vector)
{
    SteeringVector resultVector = _vector;

    double stepWidth_iteration = m_speed_mps * m_updateInterval_ms / 1000;
    double vectorLength = resultVector.norm();
    if(vectorLength>stepWidth_iteration)
    {
        resultVector /= vectorLength;
        resultVector *= stepWidth_iteration;
    }

    Coord realPosition = p_reynolds->getRealPosition();
    Coord nextPosition = Coord(realPosition.x+resultVector.x, realPosition.y+resultVector.y, realPosition.z+resultVector.z);
    p_reynolds->setRealPosition(nextPosition);

    return resultVector;
}
