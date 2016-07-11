/*
 * CSVMobilityDataAccess.cc
 *
 *  Created on: Jun 20, 2016
 *      Author: sliwa
 */

#include <CSVMobilityDataAccess.h>
#include "filehandler.h"

CSVMobilityDataAccess::CSVMobilityDataAccess(const std::string &_path) :
    m_mobilityDataIndex(0)
{
    FileHandler fileHandler;
    m_rawMobilityData = fileHandler.readLines(_path.c_str());
}

CSVMobilityDataAccess::~CSVMobilityDataAccess()
{
}

bool CSVMobilityDataAccess::updateMobilityData(int _simTime_ms)
{
    bool updated = false;
    unsigned int index = m_mobilityDataIndex;
    unsigned int oldIndex = m_mobilityDataIndex;

    while(!updated)
    {
        if(index<m_rawMobilityData.size())
        {
            MobilityData data = buildMobilityDataFromString(m_rawMobilityData.at(index));

            int simTimeMs = data.timestamp_ms;
            if(simTimeMs<=_simTime_ms && index>m_mobilityDataIndex)
            {
                m_currentMobilityData = data;
                m_mobilityDataIndex = index;
            }
            else if(simTimeMs>_simTime_ms)
            {
                m_nextMobilityData = data;
                updated = true;
            }

            index++;
        }
        else
            updated = true;
    }

    return !(m_mobilityDataIndex==oldIndex);
}

MobilityData CSVMobilityDataAccess::getCurrentMobilityData()
{
    return m_currentMobilityData;
}

MobilityData CSVMobilityDataAccess::getNextMobilityData()
{
    return m_nextMobilityData;
}

Coord CSVMobilityDataAccess::getInitialPosition()
{
    Coord position;
    if(m_rawMobilityData.size()>0)
    {
        m_currentMobilityData = buildMobilityDataFromString(m_rawMobilityData.at(0));
        position = m_currentMobilityData.position;
    }

    return position;
}

MobilityData CSVMobilityDataAccess::buildMobilityDataFromString(const std::string &_data)
{
    MobilityData data;

    std::vector<double> values = stringListToDoubleList(split(_data, ','));

    const int MIN_SIZE = 13;
    if(values.size()>=MIN_SIZE)
    {
        data.timestamp_ms = values.at(0);
        data.position = Coord(values.at(1), values.at(2), values.at(3));
        data.steeringVector = SteeringVector(values.at(4), values.at(5), values.at(6));


        Coord target(values.at(7), values.at(8), values.at(9));
        if(target.x!=-1 && target.y!=-1 && target.z!=-1)
            data.waypoints.push_back(target);

        Coord nextTarget(values.at(10), values.at(11), values.at(12));
        if(nextTarget.x!=-1 && nextTarget.y!=-1 && nextTarget.z!=-1)
            data.waypoints.push_back(nextTarget);

        data.hasSteeringVector = true;
        data.isPredicted = false;
    }

    return data;
}

std::vector<std::string> CSVMobilityDataAccess::split(const std::string &_data, const char &_separator)
{
    std::vector<std::string> items;
    std::string currentItem = "";

    for(unsigned int i=0; i<_data.size(); i++)
    {
        char character = _data.at(i);


        if(character==_separator)
        {
            items.push_back(currentItem);
            currentItem = "";
        }
        else
            currentItem += character;
    }
    items.push_back(currentItem);

    return items;
}

std::vector<double> CSVMobilityDataAccess::stringListToDoubleList(const std::vector<std::string> &_stringList)
{
    std::vector<double> doubleList;
    for(unsigned int i=0; i<_stringList.size(); i++)
        doubleList.push_back(atof(_stringList.at(i).c_str()));

    return doubleList;
}
