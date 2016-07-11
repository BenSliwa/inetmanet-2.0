//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INETMANET_2_0_LOCATIONSERVICE_H_
#define __INETMANET_2_0_LOCATIONSERVICE_H_

#include <omnetpp.h>
#include <map>

#include "Agent.h"
#include "MobilityData.h"
#include "ManetAddress.h"


class LocationService : public cSimpleModule
{
public:
    LocationService();
    ~LocationService();

    void addCurrentMobilityData(const MobilityData &_data);
    MobilityData getCurrentMobilityData();
    std::deque<MobilityData> getMobilityDataHistory();

    void addMobilityDataForAgent(const ManetAddress &_address, const MobilityData &_data);
    bool containsMobilityDataForAgent(const ManetAddress &_address);
    Agent *getAgentByAddress(const ManetAddress &_address);
    MobilityData getMobilityDataForAgent(const ManetAddress &_address);
    std::deque<MobilityData> getMobilityDataHistoryForAgent(const ManetAddress &_address);

    std::deque<Agent*> getAgents();
    int getHistoryDepth();



protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);


private:
    MobilityData m_mobilityData;
    std::deque<MobilityData> m_mobilityDataHistory;

    std::map<ManetAddress, Agent*> m_mobilityDataBase;

    int m_historyDepth;

};

#endif
