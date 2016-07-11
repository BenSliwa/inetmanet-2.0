#include "GeoDijkstra.h"
#include "limits"

GeoDijkstra::GeoDijkstra()
{
    m_inf = std::numeric_limits<int>::max();
}

GeoDijkstra::~GeoDijkstra()
{

}

std::deque<Agent*> GeoDijkstra::getDijkstraPath(Agent *_startNode, Agent *_targetNode, const std::deque<Agent*> &_agents, const std::map<Agent*,std::deque<Agent*> > &_links)
{
    m_agents = _agents;
    m_links = _links;

    std::deque<Agent*> path;

    // setup the class member variables
    m_distances.clear();
    m_nodes.clear();
    m_predecessors.clear();

    m_nodes = _agents;

    // algorithm
    init(_startNode);

    while(m_nodes.size()>0)
    {
        Agent *u = getNearestNode();
        if(!u)
            return path;

        // remove u from node set
        int index = getNodeIndex(u);
        m_nodes.erase(m_nodes.begin()+index);

        if(u==_targetNode)
        {
            path = getShortestPath(_targetNode);
            return path;
        }


        // check all node neighbors
        std::deque<Agent*> neighbors = m_links.at(u);
        for(int i=0; i<neighbors.size(); i++)
        {
            Agent *neighbor = neighbors.at(i);
            int neighborIndex = getNodeIndex(neighbor);
            if(neighborIndex>-1)    // neighbor is in the node set
                updateDistance(u, neighbor);


        }

    }

    path = getShortestPath(_targetNode);
    return path;
}


void GeoDijkstra::init(Agent *_startNode)
{
    for(int i=0; i<m_nodes.size(); i++)
    {
        Agent *node = m_nodes.at(i);

        m_distances.insert(std::pair<Agent*, double>(node, m_inf));
        m_predecessors.insert(std::pair<Agent*, Agent*>(node, 0));
    }
    m_distances.insert(std::pair<Agent*, double>(_startNode, 0));
}

void GeoDijkstra::updateDistance(Agent *_nodeU, Agent *_nodeV)
{
    double newDistance = getDistanceById(_nodeU) + getDistanceBetweenNodes(_nodeU, _nodeV);
    if(newDistance<getDistanceById(_nodeV))
    {
        m_distances.insert(std::pair<Agent*, double>(_nodeV, newDistance));
        m_predecessors.insert(std::pair<Agent*, Agent*>(_nodeV, _nodeU));
    }
}

std::deque<Agent*> GeoDijkstra::getShortestPath(Agent *_targetNode)
{
    std::deque<Agent*> path;
    path.push_back(_targetNode);

    Agent *u = _targetNode;
    while(getPredecessorById(u)!=0)
    {
        u = getPredecessorById(u);
        path.push_front(u);
    }
    return path;
}

double GeoDijkstra::getDistanceBetweenNodes(Agent *_from, Agent *_to)
{
    Coord p1 = _from->getPosition();
    Coord p2 = _to->getPosition();

    double distance = p1.distance(p2);

    return distance;
}

double GeoDijkstra::getDistanceById(Agent *_agent)
{
    return m_distances.at(_agent);
}

Agent* GeoDijkstra::getPredecessorById(Agent *_agent)
{
    return m_predecessors.at(_agent);
}

Agent* GeoDijkstra::getNearestNode()
{
    double nearestDistance = m_inf;
    Agent *nearestNode = 0;

    for(int i=0; i<m_nodes.size(); i++)
    {
        Agent *currentNode = m_nodes.at(i);
        double currentDistance = getDistanceById(currentNode);

        if(currentDistance<nearestDistance)
        {
            nearestDistance = currentDistance;
            nearestNode = currentNode;
        }
    }

    return nearestNode;
}

int GeoDijkstra::getNodeIndex(Agent *_node)
{
    int index = -1;
    for(int i=0; i<m_nodes.size(); i++)
    {
        if(m_nodes.at(i)==_node)
            index = i;
    }

    return index;
}
