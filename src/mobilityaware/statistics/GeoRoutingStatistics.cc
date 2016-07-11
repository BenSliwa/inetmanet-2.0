/*
 * GeoRoutingStatistics.cc
 *
 *  Created on: Jan 22, 2016
 *      Author: mio
 */

#include "GeoRoutingStatistics.h"
#include <iostream>


Define_Module(GeoRoutingStatistics);

GeoRoutingStatistics *g_geoRoutingStatistics = 0;

GeoRoutingStatistics::GeoRoutingStatistics() :
    m_updateMessage(0),
    m_time_s(0),
    m_fileName(""),
    m_isSaved(false)
{

}

GeoRoutingStatistics::~GeoRoutingStatistics()
{
    if(m_updateMessage)
        cancelAndDelete(m_updateMessage);



    //std::cout << "GeoRoutingStatistics::~GeoRoutingStatistics" << std::endl;
}

void GeoRoutingStatistics::initialize()
{
    //std::cout << "GeoRoutingStatistics::initialize" << std::endl;

    m_fileName = par("path").stringValue();

    g_geoRoutingStatistics = this;

    m_updateMessage = new cMessage("update");
    scheduleAt(1.0, m_updateMessage);
}

void GeoRoutingStatistics::finish()
{
    std::cout << "GeoRoutingStatistics::finish" << std::endl;

    save();
}

void GeoRoutingStatistics::handleMessage(cMessage *msg)
{
    scheduleAt(simTime()+1.0, m_updateMessage);

    update();

    m_time_s++;
}

void GeoRoutingStatistics::incSentMessages_applicationLayer(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.sentPackets_applicationLayer++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incSentMessages_applicationLayer("total");
}

void GeoRoutingStatistics::incSentBytes_applicationLayer(const std::string &_id, int _bytes)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.sentBytes_applicationLayer += _bytes;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incSentBytes_applicationLayer("total", _bytes);
}

void GeoRoutingStatistics::incEndToEndDelay_applicationLayer(const std::string &_id, int _ms)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.endToEndDelay_applicationLayer += _ms;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incEndToEndDelay_applicationLayer("total", _ms);
}

void GeoRoutingStatistics::incReceivedMessages_applicationLayer(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.receivedPackets_applicationLayer++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incReceivedMessages_applicationLayer("total");
}

void GeoRoutingStatistics::incReceivedBytes_applicationLayer(const std::string &_id, int _bytes)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.receivedBytes_applicationLayer += _bytes;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incReceivedBytes_applicationLayer("total", _bytes);
}

void GeoRoutingStatistics::incRouteChanges(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.routeChanges++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incRouteChanges("total");
}


void GeoRoutingStatistics::incSentMessages_broadcast_applicationLayer(const std::string &_id, int _agents)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.sentPackets_broadcast_applicationLayer++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incSentMessages_broadcast_applicationLayer("total", _agents);
}

void GeoRoutingStatistics::incReceivedMessages_broadcast_applicationLayer(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.receivedPackets_broadcast_applicationLayer++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incReceivedMessages_broadcast_applicationLayer("total");
}

void GeoRoutingStatistics::incSentPackets_macLayer(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.sentPackets_macLayer++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incSentPackets_macLayer("total");
}

void GeoRoutingStatistics::incReceivedPackets_macLayer(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.receivedPackets_macLayer++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incReceivedPackets_macLayer("total");
}

void GeoRoutingStatistics::incDropped(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.droppedPackets++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incDropped("total");
}

void GeoRoutingStatistics::incCollisions(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.collisions++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incCollisions("total");
}

void GeoRoutingStatistics::incBitErrors(const std::string &_id)
{
    GeoRoutingStatisticData data = getCurrentData(_id);
    data.biterrors++;

    addCurrentData(_id, data);

    //
    if(_id!="total")
        incBitErrors("total");
}


void GeoRoutingStatistics::update()
{
    //
    std::map<std::string, GeoRoutingStatisticData>::iterator it;
    for ( it = m_currentData.begin(); it != m_currentData.end(); it++ )
    {
        std::string currentId = it->first;
        GeoRoutingStatisticData currentData = it->second;

        // compute average values
        currentData.goodput_applicationLayer_Mbps = calculateThroughput(currentData);
        currentData.pdr_applicationLayer = calculatePDR(currentData);
       // if(currentData.pdr_applicationLayer>1)
       //     currentData.pdr_applicationLayer = 1;

        currentData.time_s = m_time_s;
        currentData.endToEndDelay_applicationLayer = calculateEndToEndDelay(currentData);

        //
       //std::cout << "GeoRoutingStatistics::update: " << currentId << "\tgoodput: " << currentData.goodput_applicationLayer_Mbps << "\tpdr: " << currentData.pdr_applicationLayer << std::endl;


        float broadcastPDR = 0;
        if(currentData.sentPackets_broadcast_applicationLayer>0)
            broadcastPDR = (float)currentData.receivedPackets_broadcast_applicationLayer / (float)currentData.sentPackets_broadcast_applicationLayer;

      //  if(currentId=="total")
      //      std::cout << currentData.pdr_applicationLayer << "\t" << broadcastPDR << std::endl;


        // save data to log
        if(m_dataLog.count(currentId))
            m_dataLog.at(currentId) = m_dataLog.at(currentId) + currentData.toString() + "\n";
        else
            m_dataLog.insert(std::pair<std::string, std::string>(currentId, currentData.toString() + "\n"));

    }

    m_currentData.clear();
}

GeoRoutingStatisticData GeoRoutingStatistics::getCurrentData(const std::string &_id)
{
    if(contains(_id))
        return m_currentData.at(_id);
    else
    {
        GeoRoutingStatisticData data;
        addCurrentData(_id, data);

        return data;
    }
}

void GeoRoutingStatistics::addCurrentData(const std::string &_id, GeoRoutingStatisticData _data)
{
    if(contains(_id))
        m_currentData.at(_id) = _data;
    else
        m_currentData.insert(std::pair<std::string, GeoRoutingStatisticData>(_id, _data));
}

bool GeoRoutingStatistics::contains(const std::string &_id)
{
    return m_currentData.count(_id);
}

void GeoRoutingStatistics::setFileName(const std::string &_fileName)
{
    if(m_fileName=="")
    {
        m_fileName = _fileName;
        //std::cout << "GeoRoutingStatistics::setFileName " << _fileName << std::endl;
    }
}
void GeoRoutingStatistics::save()
{
    std::cout << "GeoRoutingStatistics::save" << std::endl;

    if(!m_isSaved)
    {
        FileHandler fileHandler;

        std::map<std::string, std::string>::iterator it;
        for ( it = m_dataLog.begin(); it != m_dataLog.end(); it++ )
        {
            std::string id = it->first;
            std::string data = it->second;

            std::string path = m_fileName + "_" + id;


            if(id=="total")
                fileHandler.appendToFile(data, path);
        }

        m_isSaved = true;
    }
}

float GeoRoutingStatistics::calculateThroughput(const GeoRoutingStatisticData &_data)
{
    // throughput = bytes / time
    float data_bits = _data.receivedBytes_applicationLayer * 8;
    float time_s  = 1;

    float throughput_mbps = data_bits/ time_s /pow(10, 6);;

    return throughput_mbps;
}

float GeoRoutingStatistics::calculatePDR(const GeoRoutingStatisticData &_data)
{
    if(_data.sentPackets_applicationLayer==0)
        return 0;
    else
        return (float)_data.receivedPackets_applicationLayer/(float)_data.sentPackets_applicationLayer;
}

float GeoRoutingStatistics::calculateEndToEndDelay(const GeoRoutingStatisticData &_data)
{
   /* if(_data.receivedPackets_applicationLayer==0)
        return 0;
    else
        return (float)_data.endToEndDelay_applicationLayer/_data.receivedPackets_applicationLayer; */

    return _data.endToEndDelay_applicationLayer;
}

//GeoRoutingStatistics g_geoRoutingStatistics;
