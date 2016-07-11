/*
 * Steering.h
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#ifndef STEERING_H_
#define STEERING_H_

#include <deque>
#include "SteeringVector.h"
#include "ReynoldsMobilityModel.h"

#include "csimplemodule.h"

class Steering : public cSimpleModule
{
public:
    Steering();
    virtual ~Steering();

    void init(ReynoldsMobilityModel *_reynolds);

    double getDistance(const Coord &_from, const Coord &_to);
    Coord getTargetVector(const Coord &_from, const Coord &_to);

    std::deque<Coord> getWaypoints();

    virtual SteeringVector update() = 0;

protected:
    ReynoldsMobilityModel *p_reynolds;
    std::deque<Coord> m_waypoints;
};

#endif /* STEERING_H_ */
