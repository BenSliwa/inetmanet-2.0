/*
 * ControlledWaypoint.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#include <ControlledWaypoint.h>

Define_Module(ControlledWaypoint);

ControlledWaypoint::ControlledWaypoint() : Steering(),
    m_waypointNumber(10),
    m_perceptionRadius(50),
    m_reachedTargets(0)
{
}

ControlledWaypoint::~ControlledWaypoint()
{

}

void ControlledWaypoint::init(ReynoldsMobilityModel *_reynolds)
{
    Steering::init(_reynolds);

    updateWaypointList();
}

SteeringVector ControlledWaypoint::update()
{
    SteeringVector steeringVector;
    steeringVector.isValid = true;

    if(m_waypoints.size()>0)
    {
        Coord currentPosition = p_reynolds->getCurrentPosition();
        Coord currentWaypoint = m_waypoints.at(0);

        double distance_m = getDistance(currentPosition, currentWaypoint);
        if(distance_m<=m_perceptionRadius)
        {
            // waypoint is reached
            m_waypoints.pop_front();
            updateWaypointList();

            m_reachedTargets++;

            return update();
        }
        else
        {
            // waypoint is not reached yet
            Coord targetVector = getTargetVector(currentPosition, currentWaypoint);
            steeringVector.setValues(targetVector.x, targetVector.y, targetVector.z);
        }

        //std::cout << "d: " << distance_m << std::endl;
       // std::cout << currentPosition.x << "/" << currentPosition.y << "/" << currentPosition.z << " - " << currentWaypoint.x << "/" << currentWaypoint.y << "/" << currentWaypoint.z << std::endl;
    }


    return steeringVector;
}

void ControlledWaypoint::updateWaypointList()
{
    while(m_waypoints.size()<m_waypointNumber)
        m_waypoints.push_back(p_reynolds->createRandomWaypoint());
}

