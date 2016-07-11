/*
 * CSVSteering.cc
 *
 *  Created on: Jun 20, 2016
 *      Author: sliwa
 */

#include <CSVSteering.h>

Define_Module(CSVSteering);

CSVSteering::CSVSteering() : Steering(),
    p_dataAccess(0)
{
}

void CSVSteering::initialize()
{

}

void CSVSteering::init(CSVMobilityDataAccess *_dataAccess, ReynoldsMobilityModel *_reynolds)
{
    Steering::init(_reynolds);

    p_dataAccess = _dataAccess;
}

CSVSteering::~CSVSteering()
{
}

SteeringVector CSVSteering::update()
{
    MobilityData data = p_dataAccess->getCurrentMobilityData();
    m_waypoints = data.waypoints;
    data.steeringVector.isValid = true;

    return data.steeringVector;
}
