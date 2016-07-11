/*
 * Steering.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#include <Steering.h>

Steering::Steering() :
    p_reynolds(0)
{


}

Steering::~Steering()
{

}

void Steering::init(ReynoldsMobilityModel *_reynolds)
{
    p_reynolds = _reynolds;
}

double Steering::getDistance(const Coord &_from, const Coord &_to)
{
    return sqrt(pow(_to.x-_from.x, 2) + pow(_to.y-_from.y, 2) + pow(_to.z-_from.z, 2));
}

Coord Steering::getTargetVector(const Coord &_from, const Coord &_to)
{
    return Coord(_to.x-_from.x, _to.y-_from.y, _to.z-_from.z);
}

std::deque<Coord> Steering::getWaypoints()
{
    return m_waypoints;
}


