/*
 * CSVLocomotion.cc
 *
 *  Created on: Jun 20, 2016
 *      Author: sliwa
 */

#include <CSVLocomotion.h>

Define_Module(CSVLocomotion);

CSVLocomotion::CSVLocomotion() : Locomotion(),
    p_dataAccess(0)
{

}

CSVLocomotion::~CSVLocomotion()
{
}

void CSVLocomotion::init(CSVMobilityDataAccess *_dataAccess, ReynoldsMobilityModel *_reynolds)
{
    Locomotion::init(_reynolds);

    p_dataAccess = _dataAccess;
}

SteeringVector CSVLocomotion::handleSteeringVector(SteeringVector _vector)
{
    MobilityData currentData = p_dataAccess->getCurrentMobilityData();
    MobilityData nextData = p_dataAccess->getNextMobilityData();

    Coord previousPosition = currentData.position;
    Coord nextPosition = nextData.position;

    int currentTime_ms = simTime().dbl() * 1000;
    int previousEventTime_ms = currentData.timestamp_ms;
    int nextEventTime_ms = nextData.timestamp_ms;

    if(nextEventTime_ms!=previousEventTime_ms)
    {
        double timeFactor = (currentTime_ms-previousEventTime_ms)/(nextEventTime_ms-previousEventTime_ms);
        //if(timeFactor!=0)
        {
            Coord pos;
            pos.x = nextPosition.x + (nextPosition.x - previousPosition.x) * (timeFactor);
            pos.y = nextPosition.y + (nextPosition.y - previousPosition.y) * (timeFactor);
            pos.z = nextPosition.z + (nextPosition.z - previousPosition.z) * (timeFactor);

            // update the real agent position value
            p_reynolds->setRealPosition(pos);
        }


    }
    else
    {
    }

    return _vector;
}
