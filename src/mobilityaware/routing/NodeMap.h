/*
 * NodeMap.h
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#ifndef NODEMAP_H_
#define NODEMAP_H_

#include <map>
#include <vector>
#include "ManetAddress.h"
#include "NodeEntry.h"

class NodeMap
{
public:
    NodeMap();
    virtual ~NodeMap();

    void update();

    NodeEntry* createNodeEntry(const ManetAddress &_destination, int _sequenceNumber);
    NodeEntry* getDestinationNodeEntry(const ManetAddress &_destination);
    bool contains(const ManetAddress &_destination);

    std::vector<NodeEntry*> getEntries();

private:
    std::map<ManetAddress, NodeEntry*> m_destinations;
};

#endif /* NODEMAP_H_ */
