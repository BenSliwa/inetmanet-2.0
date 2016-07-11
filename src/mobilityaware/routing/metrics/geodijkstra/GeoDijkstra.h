/*
 * GeoDijkstra.h
 *
 *  Created on: Jul 4, 2016
 *      Author: sliwa
 */

#ifndef GEODIJKSTRA_H_
#define GEODIJKSTRA_H_

#include <deque>
#include "Agent.h"
#include "LocationService.h"


class GeoDijkstra
{
public:
    GeoDijkstra();
    virtual ~GeoDijkstra();

    std::deque<Agent*> getDijkstraPath(Agent *_startNode, Agent *_targetNode, const std::deque<Agent*> &_agents, const std::map<Agent*,std::deque<Agent*> > &_links);

private:
    // Algorithm
    void init(Agent *_startNode);
    void updateDistance(Agent *_nodeU, Agent *_nodeV);
    std::deque<Agent*> getShortestPath(Agent *_targetNode);

    // Helper methods
    double getDistanceBetweenNodes(Agent *_from, Agent *_to);
    double getDistanceById(Agent *_agent);
    Agent* getPredecessorById(Agent *_agent);
    Agent* getNearestNode();
    int getNodeIndex(Agent *_node);


private:
    std::deque<Agent*> m_agents;
    std::map<Agent*,std::deque<Agent*> > m_links;

    std::deque<Agent*> m_nodes;
    std::map<Agent*, Agent*> m_predecessors;
    std::map<Agent*, double> m_distances;

    int m_inf;

};

#endif /* GEODIJKSTRA_H_ */
