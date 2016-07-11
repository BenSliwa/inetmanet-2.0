/*
 * TrajectoryPrediction.cc
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#include "TrajectoryPrediction.h"

Define_Module(TrajectoryPrediction);

TrajectoryPrediction::TrajectoryPrediction() : cModule()
{
    m_maxSpeed_mpMs = 50 / 3600;
    m_perceptionRadius_m = 50;
    m_updateInterval_ms = 100;
}

TrajectoryPrediction::~TrajectoryPrediction()
{

}

void TrajectoryPrediction::init(LocationService *_locationService, double _speed_kmh, double _perceptionRadius_m, double _updateInterval_ms)
{
    p_locationService = _locationService;

    m_maxSpeed_mpMs = _speed_kmh / 3600;
    m_perceptionRadius_m =_perceptionRadius_m;
    m_updateInterval_ms = _updateInterval_ms;

}

void TrajectoryPrediction::initialize()
{
    m_predictionWidth = par("predictionWidth");
}

MobilityData TrajectoryPrediction::predictOwnData(int _currentTime_ms)
{
    MobilityData data;
    if(p_locationService)
    {
        std::deque<MobilityData> predictedData = predictOwn(_currentTime_ms);
        if(predictedData.size()>0)
            data = predictedData.at(predictedData.size()-1);
        else
            data = p_locationService->getCurrentMobilityData();
    }

    return data;
}

MobilityData TrajectoryPrediction::predictDataForAgent(const ManetAddress &_agent, int _currentTime_ms)
{
    MobilityData data;
    if(p_locationService)
    {
        std::deque<MobilityData> predictedData = predictForAgent(_agent, _currentTime_ms);
        if(predictedData.size()>0)
            data = predictedData.at(predictedData.size()-1);
    }

    return data;
}

std::deque<MobilityData> TrajectoryPrediction::predictForAgent(const ManetAddress &_agent, int _currentTime_ms)
{
    std::deque<MobilityData> data;
    if(p_locationService)
    {
        std::deque<MobilityData> history = p_locationService->getMobilityDataHistoryForAgent(_agent);
        data = predict(history, m_predictionWidth, _currentTime_ms);
    }

    return data;
}

std::deque<MobilityData> TrajectoryPrediction::predictOwn(int _currentTime_ms)
{
    if(p_locationService)
    {
        std::deque<MobilityData> data = p_locationService->getMobilityDataHistory();
        return predict(data, m_predictionWidth, _currentTime_ms);
    }
    else
    {
        return std::deque<MobilityData>();
    }
}

std::deque<MobilityData> TrajectoryPrediction::predict(const std::deque<MobilityData> &_historyData, int _predictionWidth, int _currentTime_ms)
{
    std::deque<MobilityData> predictedData;
    std::deque<MobilityData> historyData = _historyData;





    if(historyData.size()>0)
    {
        MobilityData lastValidData = historyData.at(historyData.size()-1);
        MobilityData currentData = lastValidData;

        if(_predictionWidth==0)
            predictedData.push_back(currentData);
        else
        {
            int time_ms = (int)(_currentTime_ms / m_updateInterval_ms) * m_updateInterval_ms;   // t<=t_current
            for(int i=0; i<_predictionWidth; i++)
            {
                time_ms += m_updateInterval_ms;

                if(currentData.hasSteeringVector)
                    currentData = predictWithSteeringVector(currentData, time_ms);
                else if(currentData.waypoints.size()>0)
                    currentData = predictWithTarget(currentData, time_ms);
                else
                    currentData = predictWithHistory(historyData, time_ms);

                //
                currentData.timestamp_ms = time_ms;
                currentData.hasSteeringVector = false;
                currentData.isPredicted = true;

                predictedData.push_back(currentData);
                historyData.push_back(currentData);
                historyData.pop_front();
            }
        }
    }

    //debugPrediction(_historyData, predictedData);

    return predictedData;
}

int TrajectoryPrediction::getPredictionWidth()
{
    return m_predictionWidth;
}


MobilityData TrajectoryPrediction::predictWithSteeringVector(MobilityData _currentData, int _nextTime_ms)
{
    MobilityData nextData = _currentData;

    double factor = double(_nextTime_ms-_currentData.timestamp_ms) / _currentData.steeringVectorInterval_ms;

    Coord sv(_currentData.steeringVector.x, _currentData.steeringVector.y, _currentData.steeringVector.z);
    Coord origin(0,0,0);

    Coord steeringVector(_currentData.steeringVector.x*factor, _currentData.steeringVector.y*factor, _currentData.steeringVector.z*factor);
    nextData.position = _currentData.position +  steeringVector;


    nextData.predictionType = "SV";
    return nextData;
}

MobilityData TrajectoryPrediction::predictWithTarget(MobilityData _currentData, int _nextTime_ms)
{
    MobilityData nextData = _currentData;
    nextData.predictionType = "T";

    // compute the next position
    Coord position = _currentData.position;
    std::deque<Coord> waypoints = _currentData.waypoints;
    Coord target = waypoints.at(0);
    nextData.position = position + (target-position)/(target.distance(position)) * (_nextTime_ms-_currentData.timestamp_ms) * m_maxSpeed_mpMs;

    // check the distance to the target
    float targetDistance_m = nextData.position.distance(target);
    if(targetDistance_m<m_perceptionRadius_m)
    {
        // perform target change
        waypoints.pop_front();
        nextData.waypoints = waypoints;
    }

    return nextData;
}

MobilityData TrajectoryPrediction::predictWithHistory(std::deque<MobilityData> _historyData, int _nextTime_ms)
{
    std::deque<MobilityData> historyData = _historyData;
    if(historyData.size()==1)
    {
        MobilityData nextData = historyData.at(historyData.size()-1);

        nextData.predictionType = "C";
        return nextData;
    }
    else
    {
        MobilityData lastValidData = _historyData.at(historyData.size()-1);

        // compute the position increment
        Coord positionIncrement;
        float totalWeight = 0;
        for(int unsigned i=1; i<_historyData.size(); i++)
        {
            MobilityData currentData = _historyData.at(i);
            MobilityData lastData = _historyData.at(i-1);

            float weight = 1;
            Coord increment = (currentData.position-lastData.position) * weight/(currentData.timestamp_ms-lastData.timestamp_ms);

            positionIncrement += increment;
            totalWeight += weight;



            //if(currentData.timestamp_ms-lastData.timestamp_ms!=100)
            //    std::cout << (currentData.timestamp_ms-lastData.timestamp_ms) << std::endl;


        }
        positionIncrement = positionIncrement/totalWeight;



        //
        MobilityData nextData = lastValidData;
        nextData.position = lastValidData.position + positionIncrement * (_nextTime_ms-lastValidData.timestamp_ms);
        nextData.predictionType = "H";
        nextData.waypoints.clear();
        return nextData;
    }
}


