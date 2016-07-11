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

#ifndef __INETMANET_2_0_REYNOLDSMOBILITYMODEL_H_
#define __INETMANET_2_0_REYNOLDSMOBILITYMODEL_H_

#include <deque>
#include "MovingMobilityBase.h"
#include "MobilityData.h"
#include "CSVMobilityDataAccess.h"
#include "ManetAddress.h"
#include "LocationService.h"

class ActionSelection;
class Steering;
class Locomotion;

class INET_API ReynoldsMobilityModel : public MovingMobilityBase
{

public:
    ReynoldsMobilityModel();

    void init(LocationService *_locationService);

    virtual Coord getCurrentPosition();

    Coord getRealPosition();
    void setRealPosition(const Coord &_position);

    Coord createNoisyPosition(double _distance_m, const Coord &_realPosition);
    Coord createRandomWaypoint();

    double getStepWidth();

    void setCurrentMobilityData(const MobilityData &_data);







protected:
    virtual void setInitialPosition();


  protected:
    virtual void initialize(int _stage);
    virtual void move();





  private:
    LocationService *p_locationService;
    ActionSelection *m_actionSelection;
    std::map<Steering*, double> m_steerings;

    Locomotion *m_locomotion;
    CSVMobilityDataAccess *m_dataAccess;

    double m_gnssNoise_m;
    double m_velocity_kmh;
    bool m_isMobile;


    MobilityData m_currentMobilityData;
};

#endif
