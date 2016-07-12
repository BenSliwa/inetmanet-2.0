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

#include "GeoDijkstraMetric.h"
#include "GeoDijkstra.h"

Define_Module(GeoDijkstraMetric);

GeoDijkstraMetric::GeoDijkstraMetric() :
        p_locationService(0),
        p_prediction(0),
        m_maxDistance_m(0)
{

}

void GeoDijkstraMetric::init(LocationService *_locationService, TrajectoryPrediction *_prediction)
{
    p_locationService = _locationService;
    p_prediction = _prediction;
}

Agent* GeoDijkstraMetric::getNextHop(Agent *_start, Agent *_destination)
{
    Agent *nextHop = 0;
    std::deque<Agent*> agents = p_locationService->getAgents();
    std::map<Agent*,std::deque<Agent*>> linkMap = buildLinkMap();

    GeoDijkstra dijkstra;
    std::deque<Agent*> path = dijkstra.getDijkstraPath(_start, _destination, agents, linkMap);

    if(path.size()>1)
        nextHop = path.at(1);

    return nextHop;
}

std::map<Agent*,std::deque<Agent*>> GeoDijkstraMetric::buildLinkMap()
{
    double time_ms = simTime().dbl() * 1000;
    std::deque<Agent*> agents = p_locationService->getAgents();
    std::map<Agent*,std::deque<Agent*>> linkMap;
    for(unsigned int i=0; i<agents.size(); i++)
    {
        Agent *from = agents.at(i);
        Coord fromPosition = p_prediction->predictDataForAgent(from->getAddress(), time_ms).position;
        std::deque<Agent*> links;

        for(unsigned int j=0; j<agents.size(); j++)
        {
            if(i!=j)
            {
                Agent *to = agents.at(j);
                Coord toPosition = p_prediction->predictDataForAgent(from->getAddress(), time_ms).position;

                double currentDistance_m = from->getPosition().distance(to->getPosition());
                double predictedDistance_m = fromPosition.distance(toPosition);

                if(predictedDistance_m<m_maxDistance_m)
                    links.push_back(to);
            }
        }

        linkMap.insert(std::pair<Agent*, std::deque<Agent*>>(from, links));
    }

    return linkMap;
}

void GeoDijkstraMetric::initialize()
{
}

