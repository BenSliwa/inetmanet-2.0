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

#include "LinkQualityService.h"
#include "math.h"

Define_Module(LinkQualityService);

void LinkQualityService::initialize()
{
}

void LinkQualityService::updateSNR(const ManetAddress &_node, double _snr)
{

}

void LinkQualityService::updateRSSI(const ManetAddress &_node, double _rssi_mW)
{
    double rssi_dBm = mW2dBm(_rssi_mW);

}

void LinkQualityService::updateLossRate(const ManetAddress &_node, double _lossRate)
{

}

double LinkQualityService::mW2dBm(double _mW)
{
    return 10*log10(_mW);
}
