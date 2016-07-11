/*
 * ControlledWaypoint.h
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#ifndef CONTROLLEDWAYPOINT_H_
#define CONTROLLEDWAYPOINT_H_

#include "Steering.h"


class ControlledWaypoint : public Steering
{
public:
    ControlledWaypoint();
    virtual ~ControlledWaypoint();

    void init(ReynoldsMobilityModel *_reynolds);

    SteeringVector update();

private:
    void updateWaypointList();


private:
    unsigned int m_waypointNumber;
    double m_perceptionRadius;

    int m_reachedTargets;
};

#endif /* CONTROLLEDWAYPOINT_H_ */
