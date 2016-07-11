//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "LocationBasedCollisionAvoidance.h"


Define_Module(LocationBasedCollisionAvoidance);

LocationBasedCollisionAvoidance::LocationBasedCollisionAvoidance() :
        p_locationService(0),
        m_minDistance_m(0)
{

}

LocationBasedCollisionAvoidance::~LocationBasedCollisionAvoidance()
{

}

void LocationBasedCollisionAvoidance::initialize()
{
    m_minDistance_m = par("minDistance").doubleValue();
}

void LocationBasedCollisionAvoidance::init(LocationService *_locationService)
{
    p_locationService = _locationService;
}

SteeringVector LocationBasedCollisionAvoidance::update()
{
    SteeringVector vector;
    vector.isValid = false;

    if(p_locationService)
    {
        Coord ownPosition = p_locationService->getCurrentMobilityData().position;
        std::deque<Agent*> agents = p_locationService->getAgents();

        for(unsigned int i=0; i<agents.size(); i++)
        {
            Agent *agent = agents.at(i);
            Coord agentPosition = agent->getPosition();
            double distance_m = ownPosition.distance(agentPosition);
            if(distance_m<m_minDistance_m)
            {
                Coord direction = ownPosition - agentPosition;
                vector += SteeringVector(direction.x, direction.y, direction.z);
                // TODO: weight with distance

                vector.isValid = true;
            }
        }
    }

    return vector;
}
