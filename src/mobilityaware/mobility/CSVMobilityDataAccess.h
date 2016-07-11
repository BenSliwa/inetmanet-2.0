/*
 * CSVMobilityDataAccess.h
 *
 *  Created on: Jun 20, 2016
 *      Author: sliwa
 */

#ifndef CSVMOBILITYDATAACCESS_H_
#define CSVMOBILITYDATAACCESS_H_

#include <vector>
#include <MobilityData.h>

class CSVMobilityDataAccess
{
public:
    CSVMobilityDataAccess(const std::string &_path);
    virtual ~CSVMobilityDataAccess();

    bool updateMobilityData(int _simTime_ms);
    MobilityData getCurrentMobilityData();
    MobilityData getNextMobilityData();

    Coord getInitialPosition();

private:
    MobilityData buildMobilityDataFromString(const std::string &_data);
    static std::vector<std::string> split(const std::string &_data, const char &_separator);
    static std::vector<double> stringListToDoubleList(const std::vector<std::string> &_stringList);

private:
    std::vector<std::string> m_rawMobilityData;

    MobilityData m_currentMobilityData;
    MobilityData m_nextMobilityData;

    int m_mobilityDataIndex;

};

#endif /* CSVMOBILITYDATAACCESS_H_ */
