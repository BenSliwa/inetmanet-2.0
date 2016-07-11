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

#ifndef __INETMANET_2_0_BATMOBILE_H_
#define __INETMANET_2_0_BATMOBILE_H_

#include "MobilityAwareRoutingBase.h"
#include "BatMobileMessage_m.h"
#include "PathScoreMetric.h"


#define BATMOBILE_PORT 254
#define BATMOBILE_TTL 5

class BatMobile : public MobilityAwareRoutingBase
{

  public:
    BatMobile();
    ~BatMobile();

  private:
    void broadcastBatMobilePacket();
    void forwardBatMobilePacket(BatMobilePacket *_packet);
    void handleBatMobilePacket(BatMobilePacket *_packet, const ManetAddress &_forwarder);

    void updateRoutingTable();

    unsigned short updateSequenceNumber();
    void debugPacket(BatMobilePacket *_packet, const ManetAddress &_forwarder);


  protected:
    virtual void handleMessage(cMessage *_msg);
    virtual int numInitStages() const { return 5; }
    virtual void initialize(int _stage);

  protected:
    virtual uint32_t getRoute(const ManetAddress &, std::vector<ManetAddress> &add){};
    virtual bool getNextHop(const ManetAddress &, ManetAddress &add, int &iface, double &val){};
    virtual void setRefreshRoute(const ManetAddress &destination, const ManetAddress & nextHop, bool isReverse) {};
    virtual bool isProactive() {return true;};
    virtual bool supportGetRoute () {return false;}
    virtual bool isOurType(cPacket * msg)
    {
        return true;
    };

    virtual bool getDestAddress(cPacket *, ManetAddress &) {return false;};


    virtual bool handleNodeStart(IDoneCallback *doneCallback){};
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback){};
    virtual void handleNodeCrash(){};

  private:
    cMessage *m_updateMessage;

    double m_broadcastInterval_s;
    unsigned short m_sequenceNumber;


};

#endif
