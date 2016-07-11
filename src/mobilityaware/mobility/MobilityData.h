/*
 * MobilityData.h
 *
 *  Created on: Jun 15, 2016
 *      Author: sliwa
 */

#ifndef MOBILITYDATA_H_
#define MOBILITYDATA_H_

#include <deque>
#include "Coord.h"
#include "SteeringVector.h"

class MobilityData {
public:
    MobilityData();
    virtual ~MobilityData();

    void output();




public:
    int timestamp_ms;
    Coord position;
    SteeringVector steeringVector;
    std::deque<Coord> waypoints;

    double steeringVectorInterval_ms;
    std::string predictionType;
    bool isPredicted;
    bool hasSteeringVector;

};

#endif /* MOBILITYDATA_H_ */
