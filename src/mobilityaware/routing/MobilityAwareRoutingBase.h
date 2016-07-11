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

#ifndef __INETMANET_2_0_MOBILITYAWAREROUTINGBASE_H_
#define __INETMANET_2_0_MOBILITYAWAREROUTINGBASE_H_


#include "ManetRoutingBase.h"
#include "ReynoldsMobilityModel.h"
#include "TrajectoryPrediction.h"
#include "LinkQualityService.h"
#include "LocationService.h"
#include "NodeMap.h"
#include "PathScoreMetric.h"

class MobilityAwareRoutingBase : public ManetRoutingBase
{
public:
    MobilityAwareRoutingBase();
    ~MobilityAwareRoutingBase();

protected:
    void broadcastPacket(cPacket *_packet, int _port, unsigned short _ttl);
    ManetAddress buildManetAddress(const std::string &_address);

    std::string getOwnHostName();
    IPvXAddress getIp(const std::string &_hostName);

    void updateRoute(NodeEntry *_entry, NeighborNodeEntry *_neighbor, int _hops);
    void changeRoute(const ManetAddress &_destination, const ManetAddress &_neighbor, int _hops);
    void invalidateRoute(const ManetAddress &_destination);



protected:
    virtual void initialize(int _stage);

  protected:
    virtual bool supportGetRoute () = 0;
    virtual uint32_t getRoute(const ManetAddress& dest, std::vector<ManetAddress>& hopsList) = 0;

    virtual bool getNextHop(const ManetAddress& dest, ManetAddress& nextHop, int& ifaceId, double& cost) = 0;
    virtual void setRefreshRoute(const ManetAddress& dest, const ManetAddress& nextHop, bool isReverse) = 0;

    virtual bool isProactive() = 0;
    virtual bool isOurType(cPacket *pk) = 0;
    virtual bool getDestAddress(cPacket *pk, ManetAddress &dest) = 0;
    virtual bool handleNodeStart(IDoneCallback *doneCallback) = 0;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) = 0;
    virtual void handleNodeCrash() = 0;

  protected:
    cModule *p_host;
    ReynoldsMobilityModel *p_mobility;
    TrajectoryPrediction *p_prediction;
    LinkQualityService *p_linkQualityService;
    LocationService *p_locationService;
    PathScoreMetric *p_metric;


    NodeMap m_nodeMap;


    std::string m_hostName;
    ManetAddress m_address;

};

#endif
