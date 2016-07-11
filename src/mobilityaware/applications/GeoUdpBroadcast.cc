#include "GeoUdpBroadcast.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"


Define_Module(GeoUdpBroadcast);

GeoUdpBroadcast::GeoUdpBroadcast() :
        p_locationService(0)
{
    //messageLengthPar = NULL;
    //burstDurationPar = NULL;
    sleepDurationPar = NULL;

    timerNext = NULL;
    addressModule = NULL;
    outputInterfaceMulticastBroadcast.clear();
}

GeoUdpBroadcast::~GeoUdpBroadcast()
{
    cancelAndDelete(timerNext);
}

void GeoUdpBroadcast::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    ApplicationBase::initialize(stage);
    if (stage == 0)
    {
        m_agents = par("agents");
        m_sequenceNumber = 0;
        m_ttl = 5;
        m_useFlooding = par("flooding").boolValue();

        delayLimit = par("delayLimit");
        startTime = par("startTime");
        stopTime = par("stopTime");

       // messageLengthPar = &par("messageLength");
       // burstDurationPar = &par("burstDuration");
        sleepDurationPar = &par("sleepDuration");
        nextSleep = startTime;
        nextBurst = startTime;
        nextPkt = startTime;
        isSource = par("isSource");


        m_sendInterval_s = par("sendInterval").doubleValue();
        m_payload = par("payload");
        m_bytesLeft = m_payload;

        m_mtu = par("mtu");

        if(m_payload==0)
            isSource = false;


        m_burstDuration = par("burstDuration").doubleValue();




        int messages = m_payload/m_mtu;
        int remaining = m_payload%m_mtu;
        if(remaining>0)
            messages++;


        m_burstDuration = messages * m_sendInterval_s - m_sendInterval_s/2;

        //std::cout << m_payload << " / " << m_mtu << " -> " << messages << std::endl;
        //std::cout << m_sendInterval_s << " - " << m_burstDuration << std::endl;
    }
    else if(stage==3)
    {
        cModule *p_host = getContainingNode(this);
        p_locationService = dynamic_cast<LocationService*>(p_host->getSubmodule("locationService"));


        processStart();
    }
  //  else if (stage ==3)

}

void GeoUdpBroadcast::processStart()
{
    localPort = par("localPort");
    destPort = par("destPort");

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    socket.setBroadcast(true);

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
    IPvXAddress myAddr = IPvXAddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
    myId = this->getParentModule()->getId();
}



GeoUdpMessage* GeoUdpBroadcast::createPacket()
{
    int data_bytes = std::min(m_mtu, m_bytesLeft);
    m_bytesLeft -= data_bytes;

    if(m_bytesLeft==0)
        m_bytesLeft = m_payload;


   // std::cout << "TX " << simTime().dbl()*1000 << "\t" << data_bytes << std::endl;

    long msgByteLength = data_bytes;
    GeoUdpMessage *payload = new GeoUdpMessage("GeoUdpBroadcast");
    payload->setByteLength(msgByteLength + 3);
    payload->setSender(getOwnIp().c_str());
    payload->setTransmissionTime_ms(simTime().dbl()*1000);
    payload->setSeq(updateSeq());
    payload->setTtl(m_ttl);

    payload->addPar("sourceId") = getId();



    MobilityData data = p_locationService->getCurrentMobilityData();
    payload->setPosition(data.position);
    payload->setSteeringVector(Coord(data.steeringVector.x, data.steeringVector.y, data.steeringVector.z));
    payload->setWaypointsArraySize(data.waypoints.size());
    for(int i=0; i<data.waypoints.size(); i++)
        payload->setWaypoints(0, data.waypoints.at(i));



    if (addressModule)
        payload->addPar("destAddr") = addressModule->choseNewModule();

    return payload;
}

void GeoUdpBroadcast::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (dynamic_cast<cPacket*>(msg))
        {
            IPvXAddress destAddr(IPv4Address::ALLONES_ADDRESS);
            sendBroadcast(destAddr, PK(msg), true);
        }
        else
        {
            if (stopTime <= 0 || simTime() < stopTime)
            {
                // send and reschedule next sending
                if (isSource) // if the node is a sink, don't generate messages
                    generateBurst();
            }
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

void GeoUdpBroadcast::processPacket(cPacket *pk)
{
    GeoUdpMessage *message = dynamic_cast<GeoUdpMessage*>(pk);
    if(message)
    {
        std::string id = message->getSender();
        int delay_ms = simTime().dbl()*1000 - message->getTransmissionTime_ms();
        int seq = message->getSeq();
        int ttl = message->getTtl();
        ttl--;

        bool updateStatistics = false;
        bool rebroadcast = false;

        if(id!=getOwnIp())
        {


            if(containsSeq(id))
            {
                int lastSeq = m_sequenceNumbers.at(id);

                if(seq>lastSeq || abs(seq-lastSeq)>100)
                {
                    m_sequenceNumbers.at(id) = seq;

                    updateStatistics = true;

                    if(ttl>0)
                        rebroadcast = true;
                }
            }
            else
            {
                m_sequenceNumbers.insert(std::pair<std::string,int>(id, seq));

                updateStatistics = true;

                if(ttl>0)
                    rebroadcast = true;
            }


            MobilityData data;
            data.position = message->getPosition();
            Coord sv = message->getSteeringVector();
            data.steeringVector = SteeringVector(sv.x, sv.y, sv.z);

            int waypointsSize = message->getWaypointsArraySize();
            std::deque<Coord> waypoints;
            for(int i=0; i<waypointsSize; i++)
                waypoints.push_back(message->getWaypoints(i));

            data.waypoints = waypoints;


            ManetAddress address;
            IPv4Address ip;
            ip.set(id.c_str());
            address.set(ip);

            p_locationService->addMobilityDataForAgent(address, data);



            if(g_geoRoutingStatistics && updateStatistics)
            {
                //g_geoRoutingStatistics->incEndToEndDelay_applicationLayer(id, delay_ms);
                //g_geoRoutingStatistics->incReceivedMessages_applicationLayer(id);
                //g_geoRoutingStatistics->incReceivedBytes_applicationLayer(id, message->getByteLength());

                g_geoRoutingStatistics->incReceivedMessages_broadcast_applicationLayer(id);
            }


            if(rebroadcast && m_useFlooding)
            {
                //std::cout << "REBROADCAST " << ttl << std::endl;


                GeoUdpMessage *msg = message->dup();
                msg->setTtl(ttl);

                IPvXAddress destAddr(IPv4Address::ALLONES_ADDRESS);
                sendBroadcast(destAddr, msg, true);
            }
        }

    }


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
        // check if this message has like origin this node


        if (moduleId == getId())
        {
            delete pk;
            return;
        }
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

    if (pk->hasPar("destAddr"))
    {
        int moduleId = (int)pk->par("destAddr");
        if (moduleId == myId)
        {
            EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

            delete pk;
            return;
        }
    }
    else
    {
        EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

    }

    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->removeControlInfo());
    if (ctrl->getDestAddr().get4() == IPv4Address::ALLONES_ADDRESS && par("flooding").boolValue())
    {
        delete pk;
      //  scheduleAt(simTime()+par("delay").doubleValue(),pk);
    }
    else
        delete pk;
    delete ctrl;

}

void GeoUdpBroadcast::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = m_sendInterval_s;
    if (sendInterval <= 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) // new burst
    {
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
    }

    IPvXAddress destAddr(IPv4Address::ALLONES_ADDRESS);

    cPacket *payload = createPacket();
    payload->setTimestamp();


    // Check address type
    sendBroadcast(destAddr, payload, true);


    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    scheduleAt(nextPkt, timerNext);
}


std::string GeoUdpBroadcast::getOwnHostName()
{
    cModule *host = getContainingNode(this);

    std::stringstream ss;
    ss << host->getName() << "[" << host->getIndex() << "]";

    return ss.str();
}

std::string GeoUdpBroadcast::getOwnIp()
{
    IPvXAddress addr = IPvXAddressResolver().resolve(getOwnHostName().c_str(),  IPvXAddressResolver::ADDR_IPv4);
    return addr.str();
}

int GeoUdpBroadcast::updateSeq()
{
    m_sequenceNumber++;
    if(m_sequenceNumber>pow(2,16)-1)
        m_sequenceNumber = 0;

    return m_sequenceNumber;
}

bool GeoUdpBroadcast::containsSeq(const std::string &_id)
{
    return m_sequenceNumbers.count(_id);
}

void GeoUdpBroadcast::finish()
{

}

bool GeoUdpBroadcast::sendBroadcast(const IPvXAddress &dest, cPacket *pkt, bool _updateStatistics)
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


            if(g_geoRoutingStatistics && _updateStatistics)
            {
                std::string ip = getOwnIp();
                //long msgByteLength = messageLengthPar->longValue();
                long msgByteLength = pkt->getByteLength();

               // std::cout << "TX" << std::endl;

                //g_geoRoutingStatistics->incSentMessages_applicationLayer(ip);
                //g_geoRoutingStatistics->incSentBytes_applicationLayer(ip, msgByteLength);

                g_geoRoutingStatistics->incSentMessages_broadcast_applicationLayer(ip, m_agents-1);
            }
        }
        return true;
    }
    return false;
}


bool GeoUdpBroadcast::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t start = std::max(startTime, simTime());

    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))
    {
        if (isSource)
        {
            activeBurst = true;
            timerNext = new cMessage("GeoUdpBroadcastTimer");
            scheduleAt(startTime, timerNext);
        }

        if (strcmp(par("destAddresses").stringValue(),"") != 0)
        {
            if (addressModule == NULL)
            {
                addressModule = new AddressModule();
                addressModule->initModule(true);
            }
        }
    }

    return true;
}

bool GeoUdpBroadcast::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void GeoUdpBroadcast::handleNodeCrash()
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
}
