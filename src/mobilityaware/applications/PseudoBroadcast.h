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

#ifndef __INETMANET_PSEUDOBROADCAST_H_
#define __INETMANET_PSEUDOBROADCAST_H_


#include <vector>
#include <map>

#include "INETDefs.h"

#include "ApplicationBase.h"
#include "UDPSocket.h"
#include "GeoUdpMessage_m.h"
#include "GeoRoutingStatistics.h"

class INET_API PseudoBroadcast : public ApplicationBase
{
  public:
    enum ChooseDestAddrMode
    {
        ONCE = 1, PER_BURST, PER_SEND
    };

  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };

    UDPSocket socket;
    int localPort, destPort;

    ChooseDestAddrMode chooseDestAddrMode;
    std::vector<IPvXAddress> destAddresses;
    IPvXAddress destAddr;
    int destAddrRNG;

    typedef std::map<int,int> SourceSequence;
    SourceSequence sourceSequence;
    simtime_t delayLimit;
    cMessage *timerNext;
    simtime_t startTime;
    simtime_t stopTime;
    simtime_t nextPkt;
    simtime_t nextBurst;
    simtime_t nextSleep;
    bool activeBurst;
    bool isSource;
    bool haveSleepDuration;
    int outputInterface;
    std::vector<int> outputInterfaceMulticastBroadcast;

    // volatile parameters:
    //cPar *messageLengthPar;
    //cPar *burstDurationPar;
    cPar *sleepDurationPar;
    //cPar *sendIntervalPar;

    // chooses random destination address
    virtual IPvXAddress chooseDestAddr();
    virtual GeoUdpMessage *createPacket();
    virtual void processPacket(cPacket *msg);
    virtual void generateBurst();

    std::string getOwnHostName();
    std::string getOwnIp();

    std::string m_hostName;
    std::string m_destinations;


    std::vector<std::string> m_targets;

    int m_messagesPerBurst;
    int m_messageIndex;
    int m_destinationIndex;

    int m_payload;
    int m_bytesLeft;
    int m_mtu;
    int m_agents;

    float m_offset;


    float m_sendInterval_s;
    float m_burstDuration;

  protected:
    virtual int numInitStages() const { return 4; }
    virtual void initialize(int stage);
    virtual void handleMessageWhenUp(cMessage *msg);
    virtual void finish();
    virtual bool sendBroadcast(const IPvXAddress &dest, cPacket *pkt);

    virtual void initialConfiguration();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();

  public:
    PseudoBroadcast();
    ~PseudoBroadcast();
};

#endif
