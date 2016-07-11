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

package inet.mobilityaware.host;

import inet.nodes.inet.AdhocHost;
import inet.mobilityaware.mobility.actionselection.IActionSelection;
import inet.mobilityaware.mobility.steering.ISteering;
import inet.mobilityaware.mobility.locomotion.ILocomotion;
import inet.mobilityaware.mobility.LocationService;
import inet.mobilityaware.mobility.prediction.TrajectoryPrediction;
import inet.mobilityaware.routing.metrics.IRoutingMetric;
import inet.mobilityaware.routing.scheduling.IRouteScheduler;
import inet.mobilityaware.routing.linkquality.LinkQualityService;


module MobilityAwareHost extends AdhocHost
{
    parameters:
        mobilityType = "ReynoldsMobilityModel";
        string actionSelectionType = default("");
        string metricType;
        string routeSchedulerType = default("");

        int numSteerings;

        
    submodules:
        locationService: LocationService {
        }
        linkQualityService: LinkQualityService {
        }
        prediction: TrajectoryPrediction {
        }
        metric: <metricType> like IRoutingMetric{
        }
        routeScheduler: <routeSchedulerType> like IRouteScheduler if routeSchedulerType != ""{
        }
        actionSelection: <actionSelectionType> like IActionSelection if actionSelectionType != ""{
        }
        steering[numSteerings]: <> like ISteering{      
        }
        locomotion: <> like ILocomotion{
        }
           
}
