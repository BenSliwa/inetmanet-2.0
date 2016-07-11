/*
 * Agent.h
 *
 *  Created on: Jul 4, 2016
 *      Author: sliwa
 */

#ifndef AGENT_H_
#define AGENT_H_

#include <deque>
#include "MobilityData.h"
#include "ManetAddress.h"
#include "Coord.h"

class Agent {
public:
    Agent(const ManetAddress &_address, int _historyDepth);
    virtual ~Agent();


    void addMobilityData(const MobilityData &_data);
    ManetAddress getAddress();
    std::deque<MobilityData> getMobilityDataHistory();
    Coord getPosition();

private:
    ManetAddress m_address;
    std::deque<MobilityData> m_mobilityHistory;
    int m_historyDepth;


};

#endif /* AGENT_H_ */
