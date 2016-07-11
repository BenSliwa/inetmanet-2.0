//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Málaga
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "GeoUdpBurst.h"
#include "configparser.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTableAccess.h"
#ifdef WITH_IPv4
#include "IRoutingTable.h"
#include "RoutingTableAccess.h"
#endif

#ifdef WITH_IPv6
#include "RoutingTable6.h"
#include "RoutingTable6Access.h"
#endif


EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("ChooseDestAddrMode");
    if (!e) enums.getInstance()->add(e = new cEnum("ChooseDestAddrMode"));
    e->insert(GeoUdpBurst::ONCE, "once");
    e->insert(GeoUdpBurst::PER_BURST, "perBurst");
    e->insert(GeoUdpBurst::PER_SEND, "perSend");
);

Define_Module(GeoUdpBurst);

GeoUdpBurst::GeoUdpBurst()
{
    //messageLengthPar = NULL;
    //burstDurationPar = NULL;
    sleepDurationPar = NULL;
    //sendIntervalPar = NULL;
    timerNext = NULL;
    outputInterface = -1;
    isSource = false;
    outputInterfaceMulticastBroadcast.clear();
}

GeoUdpBurst::~GeoUdpBurst()
{
    cancelAndDelete(timerNext);
}

void GeoUdpBurst::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == 0)
    {
        delayLimit = par("delayLimit");
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime <= startTime)
            error("Invalid startTime/stopTime parameters");

        //messageLengthPar = &par("messageLength");
        //burstDurationPar = &par("burstDuration");
        sleepDurationPar = &par("sleepDuration");

      //  sendIntervalPar = &par("sendInterval");


        nextSleep = startTime;
        nextBurst = startTime;
        nextPkt = startTime;

        destAddrRNG = par("destAddrRNG");
        const char *addrModeStr = par("chooseDestAddrMode").stringValue();
        int addrMode = cEnum::get("ChooseDestAddrMode")->lookup(addrModeStr);
        if (addrMode == -1)
            throw cRuntimeError("Invalid chooseDestAddrMode: '%s'", addrModeStr);
        chooseDestAddrMode = (ChooseDestAddrMode)addrMode;

        localPort = par("localPort");
        destPort = par("destPort");

        timerNext = new cMessage("GeoUdpBurstTimer");



        cModule *host = getContainingNode(this);
        m_offset = (float)host->getIndex() * 5 / 1000;

        //std::cout << "offset " << m_offset << std::endl;


        m_sendInterval_s = par("sendInterval").doubleValue();
        m_payload = par("payload");
        m_bytesLeft = m_payload;

        m_mtu = par("mtu");

        m_burstDuration = par("burstDuration").doubleValue();




        int messages = m_payload/m_mtu;
        int remaining = m_payload%m_mtu;
        if(remaining>0)
            messages++;



        m_burstDuration = messages * m_sendInterval_s - m_sendInterval_s/2;


        m_hostName = getOwnHostName();
        std::string hostname = m_hostName;


        std::string destinations = par("destAddresses").stringValue();

        ConfigParser configParser;
        std::map<std::string, std::string> m = configParser.key2Map(destinations, ',', '>');


        configParser.replace(hostname, "host[", "");
        configParser.replace(hostname, "]", "");

        if(m.count(hostname))
        {
            std::string targetKey = m.at(hostname);
            m_destinations = "host[" + targetKey + "]";

            std::cout << "found " << m_hostName << " -> " << m_destinations << std::endl;
        }
        else if(m.count("x"))
        {
            std::string targetKey = m.at("x");
            m_destinations = "host[" + targetKey + "]";

            std::cout << "found " << m_hostName << " -> " << m_destinations << " many to one" << std::endl;
        }
        else
        {
            std::cout << "host not found " << m_hostName << std::endl;
        }
    }
    else if (stage == 3)
    {
        if (par("configureInInit").boolValue())
            initialConfiguration();
    }
}

IPvXAddress GeoUdpBurst::chooseDestAddr()
{
    if (destAddresses.size() == 1)
        return destAddresses[0];

    int k = genk_intrand(destAddrRNG, destAddresses.size());
    return destAddresses[k];
}

GeoUdpMessage *GeoUdpBurst::createPacket()
{
    int data_bytes = std::min(m_mtu, m_bytesLeft);
    m_bytesLeft -= data_bytes;

    if(m_bytesLeft==0)
        m_bytesLeft = m_payload;

    long msgByteLength = data_bytes;

  //  std::cout << "TX " << simTime().dbl()*1000 << "\t" << data_bytes << std::endl;
  //  std::cout << getOwnIp() << std::endl;


    GeoUdpMessage *payload = new GeoUdpMessage("GeoUdpMessage");
    payload->setByteLength(msgByteLength);
    payload->setSender(getOwnIp().c_str());
    payload->setTransmissionTime_ms(simTime().dbl()*1000);


    return payload;
}

void GeoUdpBurst::initialConfiguration()
{
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    //const char *destAddrs = par("destAddresses");
    const char *destAddrs = m_destinations.c_str();


    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    bool excludeLocalDestAddresses = par("excludeLocalDestAddresses").boolValue();
    if (par("setBroadcast").boolValue())
        socket.setBroadcast(true);

    if (strcmp(par("outputInterface").stringValue(),"") != 0)
    {
        IInterfaceTable* ift = InterfaceTableAccess().get();
        InterfaceEntry *ie = ift->getInterfaceByName(par("outputInterface").stringValue());
        if (ie == NULL)
            throw cRuntimeError(this, "Invalid output interface name : %s",par("outputInterface").stringValue());
        outputInterface = ie->getInterfaceId();
    }

    outputInterfaceMulticastBroadcast.clear();
    if (strcmp(par("outputInterfaceMulticastBroadcast").stringValue(),"") != 0)
    {
        IInterfaceTable* ift = InterfaceTableAccess().get();
        const char *ports = par("outputInterfaceMulticastBroadcast");
        cStringTokenizer tokenizer(ports);
        const char *token;
        while ((token = tokenizer.nextToken()) != NULL)
        {
            if (strstr(token, "ALL") != NULL)
            {
                for (int i = 0; i < ift->getNumInterfaces(); i++)
                {
                    InterfaceEntry *ie = ift->getInterface(i);
                    if (ie->isLoopback())
                        continue;
                    if (ie == NULL)
                        throw cRuntimeError(this, "Invalid output interface name : %s", token);
                    outputInterfaceMulticastBroadcast.push_back(ie->getInterfaceId());
                }
            }
            else
            {
                InterfaceEntry *ie = ift->getInterfaceByName(token);
                if (ie == NULL)
                    throw cRuntimeError(this, "Invalid output interface name : %s", token);
                outputInterfaceMulticastBroadcast.push_back(ie->getInterfaceId());
            }
        }
    }
#ifdef WITH_IPv4
    IRoutingTable *rt = RoutingTableAccess().getIfExists();
#endif
#ifdef WITH_IPv6
    RoutingTable6 *rt6 = RoutingTable6Access().getIfExists();
#endif

    while ((token = tokenizer.nextToken()) != NULL)
    {
        if (strstr(token, "Broadcast") != NULL)
            destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
        else
        {
            IPvXAddress addr = IPvXAddressResolver().resolve(token);
#ifdef WITH_IPv4
            if (excludeLocalDestAddresses && rt && rt->isLocalAddress(addr.get4()))
                continue;
#endif
#ifdef WITH_IPv6
            if (excludeLocalDestAddresses && rt6 && rt6->isLocalAddress(addr.get6()))
                continue;
#endif
            destAddresses.push_back(addr);
        }
    }
}

void GeoUdpBurst::processStart()
{

    if (!par("configureInInit").boolValue())
        initialConfiguration();

    nextSleep = simTime();
    nextBurst = simTime();
    nextPkt = simTime();
    activeBurst = false;

    isSource = !destAddresses.empty();

    if (isSource)
    {
        if (chooseDestAddrMode == ONCE)
            destAddr = chooseDestAddr();

        activeBurst = true;
    }
    timerNext->setKind(SEND);
    processSend();
}

void GeoUdpBurst::processSend()
{
    if (stopTime < SIMTIME_ZERO || simTime() < stopTime)
    {
        // send and reschedule next sending
        if (isSource) // if the node is a sink, don't generate messages
            generateBurst();
    }
}

void GeoUdpBurst::processStop()
{
    socket.close();
}

void GeoUdpBurst::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        switch (msg->getKind()) {
            case START: processStart(); break;
            case SEND:  processSend(); break;
            case STOP:  processStop(); break;
            default: throw cRuntimeError("Invalid kind %d in self message", (int)msg->getKind());
        }
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        processPacket(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (ev.isGUI())
    {
        char buf[40];
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void GeoUdpBurst::processPacket(cPacket *pk)
{
    if (pk->getKind() == UDP_I_ERROR)
    {
        EV << "UDP error received\n";
        delete pk;
        return;
    }

    if (pk->hasPar("sourceId") && pk->hasPar("msgId"))
    {
        // duplicate control
        int moduleId = (int)pk->par("sourceId");
        int msgId = (int)pk->par("msgId");
        SourceSequence::iterator it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end())
        {
            if (it->second >= msgId)
            {
                EV << "Out of order packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

                delete pk;

                return;
            }
            else
                it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
    }

    if (delayLimit > 0)
    {
        if (simTime() - pk->getTimestamp() > delayLimit)
        {
            EV << "Old packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

            delete pk;

            return;
        }
    }

    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

    delete pk;
}

void GeoUdpBurst::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

   // double sendInterval = sendIntervalPar->doubleValue();
    double sendInterval = m_sendInterval_s;

    if (sendInterval <= 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) // new burst
    {
        //double burstDuration = burstDurationPar->doubleValue();
        double burstDuration = m_burstDuration;

        if (burstDuration < 0.0)
            throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = sleepDurationPar->doubleValue();

        if (burstDuration == 0.0)
            activeBurst = false;
        else
        {
            if (sleepDuration < 0.0)
                throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }

        if (chooseDestAddrMode == PER_BURST)
            destAddr = chooseDestAddr();
    }

    if (chooseDestAddrMode == PER_SEND)
        destAddr = chooseDestAddr();

    cPacket *payload = createPacket();
    payload->setTimestamp();


    // Check address type
    if (!sendBroadcast(destAddr, payload))
    {
        UDPSocket::SendOptions options;
        options.outInterfaceId = outputInterface;
        socket.sendTo(payload, destAddr, destPort,&options);

        if(g_geoRoutingStatistics)
        {
            std::string ip = getOwnIp();
            long msgByteLength = payload->getByteLength();

            g_geoRoutingStatistics->incSentMessages_applicationLayer(ip);
            g_geoRoutingStatistics->incSentBytes_applicationLayer(ip, msgByteLength);
        }
    }


    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    if (stopTime >= SIMTIME_ZERO && nextPkt >= stopTime)
    {
        timerNext->setKind(STOP);
        nextPkt = stopTime;
    }
    scheduleAt(nextPkt, timerNext);
}

std::string GeoUdpBurst::getOwnHostName()
{
    cModule *host = getContainingNode(this);

    std::stringstream ss;
    ss << host->getName() << "[" << host->getIndex() << "]";

    return ss.str();
}

std::string GeoUdpBurst::getOwnIp()
{
    IPvXAddress addr = IPvXAddressResolver().resolve(getOwnHostName().c_str(),  IPvXAddressResolver::ADDR_IPv4);
    return addr.str();
}


void GeoUdpBurst::finish()
{
}

bool GeoUdpBurst::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t start = std::max(startTime, simTime());

    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))
    {
        timerNext->setKind(START);
        scheduleAt(start, timerNext);
    }

    return true;
}

bool GeoUdpBurst::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void GeoUdpBurst::handleNodeCrash()
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
}

bool GeoUdpBurst::sendBroadcast(const IPvXAddress &dest, cPacket *pkt)
{
    if (!outputInterfaceMulticastBroadcast.empty() && (dest.isMulticast() || (!dest.isIPv6() && dest.get4() == IPv4Address::ALLONES_ADDRESS)))
    {
        for (unsigned int i = 0; i < outputInterfaceMulticastBroadcast.size(); i++)
        {
            UDPSocket::SendOptions options;
            options.outInterfaceId = outputInterfaceMulticastBroadcast[i];
            if (outputInterfaceMulticastBroadcast.size() - i > 1)
                socket.sendTo(pkt->dup(), dest, destPort, &options);
            else
                socket.sendTo(pkt, dest, destPort, &options);
        }
        return true;
    }
    return false;
}
