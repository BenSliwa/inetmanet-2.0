/*
 * NeighborNodeEntry.cpp
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#include <NeighborNodeEntry.h>

NeighborNodeEntry::NeighborNodeEntry(const ManetAddress &_address) :
    m_address(_address)
{

}

NeighborNodeEntry::~NeighborNodeEntry()
{
}

void NeighborNodeEntry::update()
{
    m_buffer.shift();
}

void NeighborNodeEntry::updateScore(double _score)
{
    m_buffer.updateScore(_score);
}

double NeighborNodeEntry::getScore()
{
    return m_buffer.getMean();
}

ManetAddress NeighborNodeEntry::getAddress()
{
    return m_address;
}

void NeighborNodeEntry::info()
{
    m_buffer.info();
}
