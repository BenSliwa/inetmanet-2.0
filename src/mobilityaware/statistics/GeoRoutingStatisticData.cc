/*
 * GeoRoutingStatisticData.cc
 *
 *  Created on: Jan 22, 2016
 *      Author: mio
 */

#include "GeoRoutingStatisticData.h"


GeoRoutingStatisticData::GeoRoutingStatisticData() :
    time_s(0),
    endToEndDelay_applicationLayer(0),
    sentPackets_applicationLayer(0),
    receivedPackets_applicationLayer(0),
    sentBytes_applicationLayer(0),
    receivedBytes_applicationLayer(0),
    goodput_applicationLayer_Mbps(0),
    pdr_applicationLayer(0),
    routeChanges(0),
    sentPackets_broadcast_applicationLayer(0),
    receivedPackets_broadcast_applicationLayer(0),
    sentPackets_macLayer(0),
    receivedPackets_macLayer(0),
    droppedPackets(0),
    collisions(0),
    biterrors(0)
{

}

GeoRoutingStatisticData::~GeoRoutingStatisticData()
{

}

void GeoRoutingStatisticData::incSentPackets()
{
    sentPackets_applicationLayer++;
}

void GeoRoutingStatisticData::incReceivedPackets()
{
    receivedPackets_applicationLayer++;
}

std::string GeoRoutingStatisticData::toString()
{
    float pdr_applicationLayer_normed = pdr_applicationLayer;
    if(pdr_applicationLayer_normed>1)
        pdr_applicationLayer_normed = 1;

    std::stringstream ss;
    ss << time_s << ",";
    ss << sentPackets_applicationLayer << "," << receivedPackets_applicationLayer << ",";
    ss << sentBytes_applicationLayer << "," << receivedBytes_applicationLayer << ",";
    ss << goodput_applicationLayer_Mbps << "," << pdr_applicationLayer << "," << pdr_applicationLayer_normed << ",";
    ss << endToEndDelay_applicationLayer << ",";
    ss << routeChanges << ",";
    ss << sentPackets_broadcast_applicationLayer << "," << receivedPackets_broadcast_applicationLayer << ",";
    ss << sentPackets_macLayer << "," << receivedPackets_macLayer << "," << droppedPackets << "," << collisions << "," << biterrors;

    return ss.str();
}
