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

#ifndef __INETMANET_2_0_TRAJECTORYPREDICTION_H_
#define __INETMANET_2_0_TRAJECTORYPREDICTION_H_

#include <omnetpp.h>
#include <deque>
#include "LocationService.h"



class TrajectoryPrediction : public cModule
{
public:
    TrajectoryPrediction();
    virtual ~TrajectoryPrediction();

    void init(LocationService *_locationService, double _speed_kmh, double _perceptionRadius_m, double _updateInterval_ms);

    MobilityData predictOwnData(int _currentTime_ms);
    MobilityData predictDataForAgent(const ManetAddress &_agent, int _currentTime_ms);
    std::deque<MobilityData> predictForAgent(const ManetAddress &_agent, int _currentTime_ms);
    std::deque<MobilityData> predictOwn(int _currentTime_ms);
    std::deque<MobilityData> predict(const std::deque<MobilityData> &_historyData, int _predictionWidth, int _currentTime_ms);

    int getPredictionWidth();

private:
    MobilityData predictWithSteeringVector(MobilityData _currentData, int _nextTime_ms);
    MobilityData predictWithTarget(MobilityData _currentData, int _nextTime_ms);
    MobilityData predictWithHistory(std::deque<MobilityData> _historyData, int _nextTime_ms);

protected:
    virtual void initialize();


protected:
    LocationService *p_locationService;

private:
    double m_maxSpeed_mpMs;
    double m_perceptionRadius_m;
    double m_updateInterval_ms;

    int m_predictionWidth;


};

#endif
