/*
 * GeoRoutingStatistics.h
 *
 *  Created on: Jan 22, 2016
 *      Author: mio
 */

#ifndef GEOROUTINGSTATISTICS_H_
#define GEOROUTINGSTATISTICS_H_

#include <omnetpp.h>
#include <map>
#include "filehandler.h"
#include "GeoRoutingStatisticData.h"

class GeoRoutingStatistics : public cSimpleModule
{
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

public:
    GeoRoutingStatistics();
    virtual ~GeoRoutingStatistics();



    // interface
    void incSentMessages_applicationLayer(const std::string &_id);
    void incSentBytes_applicationLayer(const std::string &_id, int _bytes);

    void incEndToEndDelay_applicationLayer(const std::string &_id, int _ms);
    void incReceivedMessages_applicationLayer(const std::string &_id);
    void incReceivedBytes_applicationLayer(const std::string &_id, int _bytes);

    void incRouteChanges(const std::string &_id);


    void incSentMessages_broadcast_applicationLayer(const std::string &_id, int _agents);
    void incReceivedMessages_broadcast_applicationLayer(const std::string &_id);

    void incSentPackets_macLayer(const std::string &_id);
    void incReceivedPackets_macLayer(const std::string &_id);
    void incDropped(const std::string &_id);
    void incCollisions(const std::string &_id);
    void incBitErrors(const std::string &_id);

    //
    void update();


    //
    GeoRoutingStatisticData getCurrentData(const std::string &_id);
    void addCurrentData(const std::string &_id, GeoRoutingStatisticData _data);
    bool contains(const std::string &_id);


    void setFileName(const std::string &_fileName);
    void save();


private:
    float calculateThroughput(const GeoRoutingStatisticData &_data);
    float calculatePDR(const GeoRoutingStatisticData &_data);
    float calculateEndToEndDelay(const GeoRoutingStatisticData &_data);


private:
    cMessage *m_updateMessage;
    int m_time_s;

    std::map<std::string, GeoRoutingStatisticData> m_currentData; //id->data (last second)
    std::map<std::string, std::string> m_dataLog; //id->data


    std::string m_fileName;
    bool m_isSaved;
};

extern GeoRoutingStatistics *g_geoRoutingStatistics;




#endif /* GEOROUTINGSTATISTICS_H_ */
