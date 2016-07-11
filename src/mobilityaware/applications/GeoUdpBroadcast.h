// Copyright (C) 2004,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __INET_GEOUDPBROADCAST_H
#define __INET_GEOUDPBROADCAST_H

#include <vector>
#include <map>

#include "INETDefs.h"

#include "ApplicationBase.h"
#include "AddressModule.h"
#include "UDPSocket.h"
#include "GeoRoutingStatistics.h"
#include "GeoUdpMessage_m.h"

#include "LocationService.h"

/**
 * UDP application. See NED for more info.
 */
class INET_API GeoUdpBroadcast : public ApplicationBase
{
protected:
  AddressModule * addressModule;
  UDPSocket socket;
  int localPort, destPort;

  int destAddrRNG;
  int myId;

  typedef std::map<int,int> SourceSequence;
  SourceSequence sourceSequence;
  simtime_t delayLimit;
  cMessage *timerNext;
  simtime_t stopTime;
  simtime_t startTime;
  simtime_t nextPkt;
  simtime_t nextBurst;
  simtime_t nextSleep;
  bool activeBurst;
  bool isSource;
  bool haveSleepDuration;
  std::vector<int> outputInterfaceMulticastBroadcast;


  // volatile parameters:
  //cPar *messageLengthPar;
  //cPar *burstDurationPar;
  cPar *sleepDurationPar;

  // chooses random destination address
  virtual GeoUdpMessage *createPacket();
  virtual void processPacket(cPacket *msg);
  virtual void generateBurst();


  std::string getOwnHostName();
  std::string getOwnIp();

  int m_payload;
  int m_bytesLeft;
  int m_mtu;


  float m_sendInterval_s;
  float m_burstDuration;

  int m_agents;
  bool m_useFlooding;
  int m_ttl;
  int m_sequenceNumber;
  std::map<std::string, int> m_sequenceNumbers;

  int updateSeq();
  bool containsSeq(const std::string &_id);

  //
  LocationService *p_locationService;


protected:
  virtual int numInitStages() const {return 4;}
  virtual void initialize(int stage);
  virtual void handleMessageWhenUp(cMessage *msg);
  virtual void finish();
  virtual bool sendBroadcast(const IPvXAddress &dest, cPacket *pkt, bool _updateStatistics);
  virtual void processStart();

  //AppBase:
  virtual bool handleNodeStart(IDoneCallback *doneCallback);
  virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
  virtual void handleNodeCrash();

public:
  GeoUdpBroadcast();
  virtual ~GeoUdpBroadcast();
};

#endif

