/*
 * Agent.cc
 *
 *  Created on: Jul 4, 2016
 *      Author: sliwa
 */

#include <Agent.h>

Agent::Agent(const ManetAddress &_address, int _historyDepth) :
    m_address(_address),
    m_historyDepth(_historyDepth)
{
}

Agent::~Agent()
{
}


void Agent::addMobilityData(const MobilityData &_data)
{
    m_mobilityHistory.push_back(_data);
    if(m_mobilityHistory.size()>m_historyDepth)
        m_mobilityHistory.pop_front();
}

ManetAddress Agent::getAddress()
{
    return m_address;
}

std::deque<MobilityData> Agent::getMobilityDataHistory()
{
    return m_mobilityHistory;
}

Coord Agent::getPosition()
{
    Coord position;

    if(m_mobilityHistory.size()>0)
    {
        MobilityData data = m_mobilityHistory.at(m_mobilityHistory.size()-1);
        position = data.position;
    }

    return position;
}
