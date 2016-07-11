//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


#include "GeoUdpSender.h"

#include "InterfaceTableAccess.h"
#include "IPvXAddressResolver.h"
#include "NodeOperations.h"
#include "UDPControlInfo_m.h"
#include "configparser.h"


Define_Module(GeoUdpSender);


GeoUdpSender::GeoUdpSender()
{
    selfMsg = NULL;
}

GeoUdpSender::~GeoUdpSender()
{
    cancelAndDelete(selfMsg);
}

void GeoUdpSender::initialize(int stage)
{
    ApplicationBase::initialize(stage);



    if (stage == 1)
    {
        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime").doubleValue();
        stopTime = par("stopTime").doubleValue();
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            error("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");


        float bitrate = par("bitrate").doubleValue();
        long messageLength_bytes = par("messageLength").longValue();

        int hostIndex = getContainingNode(this)->getIndex();
        m_offset_s = hostIndex * 10 / 1000;

        m_sendInterval_s = messageLength_bytes*8/bitrate;


        //
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

            //std::cout << "found " << m_hostName << " -> " << m_destinations << std::endl;
        }
        else
        {
            //std::cout << "host not found " << m_hostName << std::endl;
        }

    }
}

void GeoUdpSender::finish()
{
    ApplicationBase::finish();
}

void GeoUdpSender::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0])
    {
        IInterfaceTable *ift = InterfaceTableAccess().get(this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups)
        socket.joinLocalMulticastGroups();
}


std::string GeoUdpSender::getOwnHostName()
{
    cModule *host = getContainingNode(this);

    std::stringstream ss;
    ss << host->getName() << "[" << host->getIndex() << "]";

    return ss.str();
}

std::string GeoUdpSender::getOwnIp()
{
    IPvXAddress addr = IPvXAddressResolver().resolve(getOwnHostName().c_str(),  IPvXAddressResolver::ADDR_IPv4);
    return addr.str();
}


IPvXAddress GeoUdpSender::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isIPv6() && destAddresses[k].get6().isLinkLocal()) // KLUDGE for IPv6
    {
        //const char *destAddrs = par("destAddresses");
        const char *destAddrs = m_destinations.c_str();

        cStringTokenizer tokenizer(destAddrs);
        const char *token;

        for (int i = 0; i <= k; ++i)
            token = tokenizer.nextToken();
        destAddresses[k] = IPvXAddressResolver().resolve(token);
    }
    return destAddresses[k];
}

void GeoUdpSender::sendPacket()
{
    long msgByteLength = par("messageLength").longValue();

    GeoUdpMessage *payload = new GeoUdpMessage("GeoUdpMessage");
    payload->setByteLength(msgByteLength);
    payload->setSender(getOwnIp().c_str());
    payload->setTransmissionTime_ms(simTime().dbl()*1000);

    IPvXAddress destAddr = chooseDestAddr();


    socket.sendTo(payload, destAddr, destPort);

    if(g_geoRoutingStatistics)
    {
        std::string ip = getOwnIp();

        g_geoRoutingStatistics->incSentMessages_applicationLayer(ip);
        g_geoRoutingStatistics->incSentBytes_applicationLayer(ip, msgByteLength);
    }

}

void GeoUdpSender::processStart()
{
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    setSocketOptions();

    //const char *destAddrs = par("destAddresses");
    const char *destAddrs = m_destinations.c_str();

    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != NULL) {
        IPvXAddress result;
        IPvXAddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV << "cannot resolve destination address: " << token << endl;
        else
            destAddresses.push_back(result);
    }

    if (!destAddresses.empty())
    {
        selfMsg->setKind(SEND);
        processSend();
    }
    else
    {
        if (stopTime >= SIMTIME_ZERO)
        {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

void GeoUdpSender::processSend()
{
    sendPacket();
    simtime_t d = simTime() + m_sendInterval_s + m_offset_s;
    if (stopTime < SIMTIME_ZERO || d < stopTime)
    {
        selfMsg->setKind(SEND);
        scheduleAt(d, selfMsg);
    }
    else
    {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void GeoUdpSender::processStop()
{
    socket.close();
}

void GeoUdpSender::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START: processStart(); break;
            case SEND:  processSend(); break;
            case STOP:  processStop(); break;
            default: throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
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

void GeoUdpSender::processPacket(cPacket *pk)
{
    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
}

bool GeoUdpSender::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))
    {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
    return true;
}

bool GeoUdpSender::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void GeoUdpSender::handleNodeCrash()
{
    if (selfMsg)
        cancelEvent(selfMsg);
}

