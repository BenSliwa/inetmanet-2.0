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

#include "LocationService.h"

Define_Module(LocationService);

LocationService::LocationService()
{

}

LocationService::~LocationService()
{
    std::map<ManetAddress, Agent*>::iterator it;
    for ( it = m_mobilityDataBase.begin(); it != m_mobilityDataBase.end(); it++ )
    {
        Agent *agent = it->second;
        delete agent;
    }
}

void LocationService::initialize()
{
    m_historyDepth = par("historyDepth");
}

void LocationService::handleMessage(cMessage *msg)
{

}

void LocationService::addCurrentMobilityData(const MobilityData &_data)
{
    m_mobilityData = _data;
    m_mobilityDataHistory.push_back(m_mobilityData);
    if(m_mobilityDataHistory.size()>m_historyDepth)
        m_mobilityDataHistory.pop_front();
}

MobilityData LocationService::getCurrentMobilityData()
{
    return m_mobilityData;
}

std::deque<MobilityData> LocationService::getMobilityDataHistory()
{
    std::deque<MobilityData> history;
    if(m_mobilityDataHistory.size()>0)
        history = m_mobilityDataHistory;
    else
        history.push_back(m_mobilityData);

    return history;
}

void LocationService::addMobilityDataForAgent(const ManetAddress &_address, const MobilityData &_data)
{
    Agent *agent = getAgentByAddress(_address);
    if(!agent)
    {
        agent = new Agent(_address, m_historyDepth);
        agent->addMobilityData(_data);

        m_mobilityDataBase.insert(std::pair<ManetAddress,Agent*>(_address, agent));
    }
    else
    {
        agent->addMobilityData(_data);
    }
}

bool LocationService::containsMobilityDataForAgent(const ManetAddress &_address)
{
    return m_mobilityDataBase.count(_address);
}

Agent* LocationService::getAgentByAddress(const ManetAddress &_address)
{
    Agent *agent = 0;
    if(containsMobilityDataForAgent(_address))
        agent = m_mobilityDataBase.at(_address);

    return agent;
}

MobilityData LocationService::getMobilityDataForAgent(const ManetAddress &_address)
{
    MobilityData data;
    std::deque<MobilityData> history = getMobilityDataHistoryForAgent(_address);
    if(history.size()>0)
        data = history.at(history.size()-1);

    return data;
}

std::deque<MobilityData> LocationService::getMobilityDataHistoryForAgent(const ManetAddress &_address)
{
    std::deque<MobilityData> history;
    Agent *agent = getAgentByAddress(_address);
    if(agent)
        history = agent->getMobilityDataHistory();

    return history;
}

std::deque<Agent*> LocationService::getAgents()
{
    std::deque<Agent*> agents;
    std::map<ManetAddress, Agent*>::iterator it;
    for ( it = m_mobilityDataBase.begin(); it != m_mobilityDataBase.end(); it++ )
    {
        Agent *agent = it->second;
        agents.push_back(agent);
    }

    return agents;
}

int LocationService::getHistoryDepth()
{
    return m_historyDepth;
}
