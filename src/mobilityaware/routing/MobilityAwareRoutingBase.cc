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

#include "MobilityAwareRoutingBase.h"
#include "IPvXAddressResolver.h"

MobilityAwareRoutingBase::MobilityAwareRoutingBase() :
    p_mobility(0),
    p_prediction(0),
    p_locationService(0)
{

}

MobilityAwareRoutingBase::~MobilityAwareRoutingBase()
{

}

void MobilityAwareRoutingBase::broadcastPacket(cPacket *_packet, int _port, unsigned short _ttl)
{
    ManetAddress address = getAddress();
    ManetAddress destination = buildManetAddress("255.255.255.255");

    int delay = 0;
    sendToIp(_packet, _port, destination, _port, _ttl, delay, address);
}


ManetAddress MobilityAwareRoutingBase::buildManetAddress(const std::string &_address)
{
    ManetAddress address;
    if(_address!="")
    {
        IPv4Address ip;
        ip.set(_address.c_str());
        address.set(ip);
    }

    return address;
}

std::string MobilityAwareRoutingBase::getOwnHostName()
{
    std::stringstream ss;
    ss << p_host->getName() << "[" << p_host->getIndex() << "]";

    return ss.str();
}

IPvXAddress MobilityAwareRoutingBase::getIp(const std::string &_hostName)
{
    IPvXAddress addr = IPvXAddressResolver().resolve(_hostName.c_str(),  IPvXAddressResolver::ADDR_IPv4);
    return addr;
}

void MobilityAwareRoutingBase::updateRoute(NodeEntry *_entry, NeighborNodeEntry *_neighbor, int _hops)
{
    if(_entry)
    {
        if(_neighbor)
            changeRoute(_entry->getAddress(), _neighbor->getAddress(), _hops);
        else
            invalidateRoute(_entry->getAddress());
    }
}

void MobilityAwareRoutingBase::changeRoute(const ManetAddress &_destination, const ManetAddress &_neighbor, int _hops)
{
    ManetAddress netmask = buildManetAddress("255.255.255.255");
    omnet_chg_rte(_destination, _neighbor, netmask, _hops, false);
}

void MobilityAwareRoutingBase::invalidateRoute(const ManetAddress &_destination)
{
    ManetAddress netmask = buildManetAddress("255.255.255.255");
    omnet_chg_rte(_destination, ManetAddress::ZERO, netmask, 1, true);
}

void MobilityAwareRoutingBase::initialize(int _stage)
{
    ManetRoutingBase::initialize(_stage);
    if(_stage==4)
    {
        p_host = getContainingNode(this);

        p_mobility = dynamic_cast<ReynoldsMobilityModel*>(p_host->getSubmodule("mobility"));
        p_linkQualityService = dynamic_cast<LinkQualityService*>(p_host->getSubmodule("linkQualityService"));
        p_locationService = dynamic_cast<LocationService*>(p_host->getSubmodule("locationService"));
        p_prediction = dynamic_cast<TrajectoryPrediction*>(p_host->getSubmodule("prediction"));
        p_metric = dynamic_cast<PathScoreMetric*>(p_host->getSubmodule("metric"));

        p_mobility->init(p_locationService);

        m_address = getAddress();
    }
}
