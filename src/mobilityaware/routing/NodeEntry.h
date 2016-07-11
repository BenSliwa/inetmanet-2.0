/*
 * NodeEntry.h
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#ifndef NODEENTRY_H_
#define NODEENTRY_H_


#include <map>
#include "ManetAddress.h"
#include "NeighborNodeEntry.h"

class NodeEntry
{
public:
    NodeEntry(const ManetAddress &_address, int _sequenceNumber);
    virtual ~NodeEntry();

    void update();

    NeighborNodeEntry* getBestNeighbor();

    bool updateSequenceNumber(unsigned short _sequenceNumber);
    void updateNeighborScore(const ManetAddress &_neighbor, double _score);

    ManetAddress getAddress();
    unsigned short getSequenceNumber();


    NeighborNodeEntry* getNeighborNodeEntry(const ManetAddress &_neighbor);
    bool contains(const ManetAddress &_destination);

private:
    std::map<ManetAddress, NeighborNodeEntry*> m_neighbors;

public:
    ManetAddress m_address;
    int m_sequenceNumber;

    NeighborNodeEntry *m_currentBestNeighbor;
};

#endif /* NODEENTRY_H_ */
