//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.mobilityaware.applications;

import inet.applications.IUDPApp;



//
// Sends UDP packets to the given IP address at the given interval.
// Compatible with both ~IPv4 and ~IPv6.
//
// The sending interval can be a constant or a random value (e.g. exponential(1)).
// If the destAddresses parameter contains more than one address, one
// of them is randomly chosen for each packet. An address may be given in the
// dotted decimal notation, or with the module name. (The IPvXAddressResolver
// class is used to resolve the address.) To disable the model, set
// destAddresses to "".
//
// Received packets are discarded.
//
// The peer can be a ~UDPSink, another ~UDPBasicApp (it handles received packets
// like ~UDPSink), or a ~UDPEchoApp. When used with ~UDPEchoApp, the rcvdPkLifetime
// statistic will contain the round-trip times.
//
simple GeoUdpBroadcast like IUDPApp
{
    parameters:        
        int localPort = default(-1);  // local UDP port number (-1: use ephemeral port)
        int destPort; // remote UDP port number

        int agents;
        
        int payload;
        int mtu;
        
        double startTime @unit(s) = default(0s); // application start time (start of the first burst)
        double stopTime @unit(s) = default(-1s); // application stop time (no packets are sent after this time); a negative or zero value means no limit


        
        volatile double burstDuration @unit(s) = default(1s); // burst duration time (zero not allowed)
        volatile double sleepDuration @unit(s); // time between bursts (zero allowed)
        volatile double sendInterval @unit(s); // time between messages during bursts; usually a random value, e.g. 0.1s+uniform(-0.001s,0.001s); zero not allowed
        
        double delayLimit @unit(s) = default(0); // maximum accepted delay for a packet; packets with a bigger delay are discarded (dropped), zero value means no limit 
        string outputInterfaceMulticastBroadcast = default("ALL"); // select the interface in which the broadcast packets will be sent
        string destAddresses = default(""); 
        bool isSource = default(false);
        bool flooding = default(false);
        volatile double delay  @unit(s) = default(uniform(0s,0.1s)); 

        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
