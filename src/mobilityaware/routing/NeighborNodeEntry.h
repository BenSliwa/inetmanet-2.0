/*
 * NeighborNodeEntry.h
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#ifndef NEIGHBORNODEENTRY_H_
#define NEIGHBORNODEENTRY_H_

#include "ManetAddress.h"
#include "NeighborScoreBuffer.h"

class NeighborNodeEntry
{
public:
    NeighborNodeEntry(const ManetAddress &_address);
    virtual ~NeighborNodeEntry();

    void update();
    void updateScore(double _score);
    double getScore();

    ManetAddress getAddress();

    void info();

private:
    NeighborScoreBuffer m_buffer;
    ManetAddress m_address;

};

#endif /* NEIGHBORNODEENTRY_H_ */
