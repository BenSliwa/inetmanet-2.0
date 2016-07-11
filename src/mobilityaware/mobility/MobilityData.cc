/*
 * MobilityData.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#include <MobilityData.h>
#include <iostream>

MobilityData::MobilityData() :
    isPredicted(false),
    hasSteeringVector(false)
{
}

MobilityData::~MobilityData()
{
}

void MobilityData::output()
{
    std::cout << "t: " << timestamp_ms << "\tp: " << predictionType << std::endl;
    std::cout << "pos: (" << position.x << "," << position.y << "," << position.z << ") sv: ("  << steeringVector.x << "," << steeringVector.y << "," << steeringVector.z << ")" << std::endl;
}
