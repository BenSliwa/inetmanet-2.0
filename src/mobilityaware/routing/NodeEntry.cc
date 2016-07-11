/*
 * NodeEntry.cc
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#include <NodeEntry.h>
#include <limits>

NodeEntry::NodeEntry(const ManetAddress &_address, int _sequenceNumber) :
    m_address(_address),
    m_sequenceNumber(_sequenceNumber),
    m_currentBestNeighbor(0)
{

}

NodeEntry::~NodeEntry()
{
    std::map<ManetAddress, NeighborNodeEntry*>::iterator it;
    for ( it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        NeighborNodeEntry *entry = it->second;
        delete entry;
    }
}

void NodeEntry::update()
{
    std::map<ManetAddress, NeighborNodeEntry*>::iterator it;
    for ( it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        NeighborNodeEntry *entry = it->second;
        entry->update();
    }
}

NeighborNodeEntry* NodeEntry::getBestNeighbor()
{
    NeighborNodeEntry *bestNeighbor = 0;

    //
    double bestScore =  std::numeric_limits<int>::min();
    std::map<ManetAddress, NeighborNodeEntry*>::iterator it;
    for ( it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        NeighborNodeEntry *entry = it->second;
        double score = entry->getScore();

        if(score>bestScore)
        {
            bestScore = score;
            bestNeighbor = entry;
        }

    }

    // select the current best neighbor if scores are equal to avoid route switches
    if(m_currentBestNeighbor)
    {
        if(m_currentBestNeighbor->getScore()==bestScore)
            bestNeighbor = m_currentBestNeighbor;
    }

    m_currentBestNeighbor = bestNeighbor;

    return bestNeighbor;
}

bool NodeEntry::updateSequenceNumber(unsigned short _sequenceNumber)
{
    bool updated = (_sequenceNumber!=m_sequenceNumber);
    m_sequenceNumber = _sequenceNumber;

    return updated;
}

void NodeEntry::updateNeighborScore(const ManetAddress &_neighbor, double _score)
{
    NeighborNodeEntry *neighbor = getNeighborNodeEntry(_neighbor);
    neighbor->updateScore(_score);
}

ManetAddress NodeEntry::getAddress()
{
    return m_address;
}

unsigned short NodeEntry::getSequenceNumber()
{
    return m_sequenceNumber;
}

NeighborNodeEntry* NodeEntry::getNeighborNodeEntry(const ManetAddress &_neighbor)
{
    NeighborNodeEntry *entry = 0;
    if(contains(_neighbor))
        entry = m_neighbors.at(_neighbor);
    else
    {
        entry = new NeighborNodeEntry(_neighbor);
        m_neighbors.insert(std::pair<ManetAddress, NeighborNodeEntry*>(_neighbor, entry));
    }

    return entry;
}

bool NodeEntry::contains(const ManetAddress &_neighbor)
{
    return m_neighbors.count(_neighbor);
}
