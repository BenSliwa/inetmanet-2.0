/*
 * NodeMap.cc
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#include <NodeMap.h>

NodeMap::NodeMap()
{


}

NodeMap::~NodeMap()
{
    std::map<ManetAddress, NodeEntry*>::iterator it;
    for ( it = m_destinations.begin(); it != m_destinations.end(); it++ )
    {
        NodeEntry *entry = it->second;
        delete entry;
    }
}

void NodeMap::update()
{
    std::map<ManetAddress, NodeEntry*>::iterator it;
    for ( it = m_destinations.begin(); it != m_destinations.end(); it++ )
    {
        NodeEntry *entry = it->second;
        entry->update();
    }
}

NodeEntry* NodeMap::createNodeEntry(const ManetAddress &_destination, int _sequenceNumber)
{
    NodeEntry *node = new NodeEntry(_destination, _sequenceNumber);
    m_destinations.insert(std::pair<ManetAddress, NodeEntry*>(_destination, node));

    return node;
}

NodeEntry* NodeMap::getDestinationNodeEntry(const ManetAddress &_destination)
{
    NodeEntry *entry = 0;
    if(contains(_destination))
        entry = m_destinations.at(_destination);

    return entry;
}

bool NodeMap::contains(const ManetAddress &_destination)
{
    return m_destinations.count(_destination);
}

std::vector<NodeEntry*> NodeMap::getEntries()
{
    std::vector<NodeEntry*> entries;
    std::map<ManetAddress, NodeEntry*>::iterator it;
    for ( it = m_destinations.begin(); it != m_destinations.end(); it++ )
        entries.push_back(it->second);

    return entries;
}
