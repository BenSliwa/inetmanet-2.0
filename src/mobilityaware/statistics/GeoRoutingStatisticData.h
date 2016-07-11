/*
 * GeoRoutingStatisticData.h
 *
 *  Created on: Jan 22, 2016
 *      Author: mio
 */

#ifndef GEOROUTINGSTATISTICDATA_H_
#define GEOROUTINGSTATISTICDATA_H_

#include <sstream>

class GeoRoutingStatisticData {
public:
    GeoRoutingStatisticData();
    virtual ~GeoRoutingStatisticData();

    void incSentPackets();
    void incReceivedPackets();

    std::string toString();

public:
    int time_s;


    int sentPackets_applicationLayer;
    int receivedPackets_applicationLayer;
    int sentBytes_applicationLayer;
    int receivedBytes_applicationLayer;

    float goodput_applicationLayer_Mbps;
    float pdr_applicationLayer;

    int endToEndDelay_applicationLayer;
    int routeChanges;


    int sentPackets_broadcast_applicationLayer;
    int receivedPackets_broadcast_applicationLayer;

    // mac
    int sentPackets_macLayer;
    int receivedPackets_macLayer;
    int droppedPackets;
    int collisions;
    int biterrors;

};

#endif /* GEOROUTINGSTATISTICDATA_H_ */
