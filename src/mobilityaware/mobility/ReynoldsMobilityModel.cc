//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "ReynoldsMobilityModel.h"


#include "ActionSelection.h"
#include "Steering.h"
#include "Locomotion.h"

#include "ControlledWaypoint.h"
#include "CSVSteering.h"
#include "LocationBasedCollisionAvoidance.h"

#include "CSVLocomotion.h"
#include "UAVLocomotion.h"


#include "cmodule.h"


Define_Module(ReynoldsMobilityModel);

ReynoldsMobilityModel::ReynoldsMobilityModel() : MovingMobilityBase(),
        p_locationService(0),
        m_actionSelection(0),
        m_locomotion(0),
        m_dataAccess(0)
{

}

ReynoldsMobilityModel::~ReynoldsMobilityModel()
{
    if(m_actionSelection)
        delete m_actionSelection;

    std::map<Steering*, double>::iterator it;
    for ( it = m_steerings.begin(); it != m_steerings.end(); it++ )
    {
        Steering *steering = it->first;
        delete steering;
    }

    if(m_locomotion)
        delete m_locomotion;

    if(m_dataAccess)
        delete m_dataAccess;
}

void ReynoldsMobilityModel::init(LocationService *_locationService)
{
    p_locationService = _locationService;
    p_locationService->addCurrentMobilityData(m_currentMobilityData);
}

Coord ReynoldsMobilityModel::getCurrentPosition()
{
    return m_currentMobilityData.position;
}

Coord ReynoldsMobilityModel::getRealPosition()
{
    return lastPosition;
}

void ReynoldsMobilityModel::setRealPosition(const Coord &_position)
{
    lastPosition = _position;
}
Coord ReynoldsMobilityModel::createNoisyPosition(double _distance_m, const Coord &_realPosition)
{
    double theta = uniform(0, 360);
    double phi = uniform(0, 360);
    double r = uniform(0, _distance_m);

    Coord noise(r*sin(theta)*cos(phi), r*sin(theta)*sin(phi), r*cos(theta));
    Coord position = _realPosition + noise;

    return position;
}

Coord ReynoldsMobilityModel::createRandomWaypoint()
{
    return getRandomPosition();
}

double ReynoldsMobilityModel::getStepWidth()
{
    double speed_mps = m_velocity_kmh/3.6;
    double updateInterval_ms = 100;
    double stepWidth_iteration = speed_mps * updateInterval_ms / 1000;

    return stepWidth_iteration;
}

void ReynoldsMobilityModel::setInitialPosition()
{
    if(!m_isMobile)
    {
        lastPosition.x = par("initialX");
        lastPosition.y = par("initialY");
    }
    else if(m_dataAccess)
    {
        Coord position = m_dataAccess->getInitialPosition();
        setRealPosition(position);
    }
    else
    {
        lastPosition = getRandomPosition();
    }

    m_currentMobilityData.timestamp_ms = 0;
    m_currentMobilityData.position = createNoisyPosition(m_gnssNoise_m, lastPosition);
}

void ReynoldsMobilityModel::setCurrentMobilityData(const MobilityData &_data)
{
    m_currentMobilityData = _data;
}


void ReynoldsMobilityModel::initialize(int _stage)
{

    MovingMobilityBase::initialize(_stage);

    if(_stage==0)
    {
        p_locationService = dynamic_cast<LocationService*>(getContainingNode(this)->getSubmodule("locationService"));

        m_gnssNoise_m = par("maxPositioningError");
        m_velocity_kmh = par("velocity").doubleValue();
        m_isMobile = par("isMobile").boolValue();

        // init steerings
        cModule *host = getContainingNode(this);
        int numSteerings = host->par("numSteerings");
        for(int i=0; i<numSteerings; i++)
        {

            cModule *steering = host->getSubmodule("steering", i);
            double weight = steering->par("weight").doubleValue();

            ControlledWaypoint *controlledWaypoint = dynamic_cast<ControlledWaypoint*>(steering);
            CSVSteering *csvSteering = dynamic_cast<CSVSteering*>(steering);
            LocationBasedCollisionAvoidance *locationBasedCA = dynamic_cast<LocationBasedCollisionAvoidance*>(steering);

            if(controlledWaypoint)
            {
                controlledWaypoint->init(this);

                m_steerings.insert(std::pair<Steering*,double>(controlledWaypoint, weight));
            }
            else if(csvSteering)
            {
                cModule *host = getContainingNode(this);
                int hostIndex = host->getIndex();
                hostIndex++;

                std::string baseDir = csvSteering->par("csvPath").stringValue();
                std::string currentRun = csvSteering->par("csvRun").stringValue();
                std::string path = baseDir + currentRun;

                std::stringstream stream;
                stream << path << "/Agent" << hostIndex << ".txt";

                m_dataAccess = new CSVMobilityDataAccess(stream.str());
                csvSteering->init(m_dataAccess, this);

                m_steerings.insert(std::pair<Steering*,double>(csvSteering, weight));
            }
            else if(locationBasedCA)
            {
                locationBasedCA->init(p_locationService);
                m_steerings.insert(std::pair<Steering*,double>(locationBasedCA, weight));

            }
        }

        // init locomotion
        cModule *locomotion = host->getSubmodule("locomotion");
        CSVLocomotion *csvLocomotion = dynamic_cast<CSVLocomotion*>(locomotion);
        UAVLocomotion *uavLocomotion = dynamic_cast<UAVLocomotion*>(locomotion);
        if(csvLocomotion)
        {
            csvLocomotion->init(m_dataAccess, this);
            m_locomotion = csvLocomotion;
        }
        else if(uavLocomotion)
        {
            uavLocomotion->init(m_velocity_kmh, 100, this);
            m_locomotion = uavLocomotion;
        }

        p_locationService->addCurrentMobilityData(m_currentMobilityData);
    }



}

void ReynoldsMobilityModel::move()
{
    if(m_isMobile)
    {
        //
        bool positionUpdated = true;
        if(m_dataAccess)
            positionUpdated = m_dataAccess->updateMobilityData(simTime().dbl() * 1000);

        // update the real agent position
        std::map<Steering*, double>::iterator it;
        std::deque<Coord> totalWaypoints;
        double totalWeight = 0;

        SteeringVector totalSteeringVector;
        for ( it = m_steerings.begin(); it != m_steerings.end(); it++ )
        {
            Steering *steering = it->first;
            double weight = it->second;

            SteeringVector steeringVector = steering->update();
            if(steeringVector.isValid)
            {
                totalWeight += weight;
                totalSteeringVector += steeringVector;
                totalSteeringVector *= weight;
            }

            std::deque<Coord> waypoints = steering->getWaypoints();
            totalWaypoints.insert(totalWaypoints.end(), waypoints.begin(), waypoints.end());

        }

        totalSteeringVector = m_locomotion->handleSteeringVector(totalSteeringVector);

        // update the mobility data
        if(positionUpdated)
        {
            m_currentMobilityData.timestamp_ms = simTime().dbl() * 1000;
            m_currentMobilityData.position = createNoisyPosition(m_gnssNoise_m, lastPosition);
            m_currentMobilityData.steeringVector = totalSteeringVector;
            m_currentMobilityData.steeringVectorInterval_ms = 100;
            m_currentMobilityData.hasSteeringVector = true;
            m_currentMobilityData.waypoints = totalWaypoints;

            p_locationService->addCurrentMobilityData(m_currentMobilityData);
        }
    }
    else
    {
        // gps noise
        m_currentMobilityData.timestamp_ms = simTime().dbl() * 1000;
        p_locationService->addCurrentMobilityData(m_currentMobilityData);
    }

}




