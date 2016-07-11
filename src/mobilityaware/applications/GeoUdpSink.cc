#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "GeoUdpSink.h"
#include "UDPControlInfo_m.h"

Define_Module(GeoUdpSink);

void GeoUdpSink::initialize()
{
    setSocketOptions();
}

void GeoUdpSink::handleMessage(cMessage *msg)
{
    GeoUdpMessage *packet = dynamic_cast<GeoUdpMessage*>(msg);
    if(packet)
        processPacket(packet);
    else
        delete packet;
}

void GeoUdpSink::setSocketOptions()
{
    int port = par("port");
    m_isPseudoBroadcast = par("isPseudoBroadcast").boolValue();

    socket.setOutputGate(gate("udpOut"));
    socket.bind(port);

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

void GeoUdpSink::processPacket(GeoUdpMessage *_message)
{
    std::string id = _message->getSender();
    long delay_ms = simTime().dbl()*1000 - _message->getTransmissionTime_ms();

    if(g_geoRoutingStatistics)
    {
        if(m_isPseudoBroadcast)
        {
            g_geoRoutingStatistics->incReceivedMessages_broadcast_applicationLayer(id);
        }
        else
        {
            g_geoRoutingStatistics->incEndToEndDelay_applicationLayer(id, delay_ms);
            g_geoRoutingStatistics->incReceivedMessages_applicationLayer(id);
            g_geoRoutingStatistics->incReceivedBytes_applicationLayer(id, _message->getByteLength());
        }

    }

    delete _message;
}

