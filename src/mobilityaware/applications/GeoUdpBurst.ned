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

package inet.mobilityaware.applications;
import inet.applications.IUDPApp;

simple GeoUdpBurst like IUDPApp
{
    parameters:
        int payload;
        int mtu;
               
        string destAddresses; // space-separated list of destination IP addresses, can be empty;
                              // see module description for possibilities
        bool excludeLocalDestAddresses = default(true);     // excluding local addresses from destAddresses
        string chooseDestAddrMode @enum("once","perBurst","perSend"); // when to change the destination address
        int destAddrRNG = default(0); // random generator ID for selecting a destination address
        int localPort = default(-1);  // local UDP port number (-1: use ephemeral port)
        int destPort; // remote UDP port number
        double startTime @unit(s) = default(0s); // application start time (start of the first burst)
        double stopTime @unit(s) = default(-1s); // application stop time (no packets are sent after this time); a negative or -1 means no limit
       // volatile int messageLength @unit(B); // length of messages to generate, in bytes
        volatile double burstDuration @unit(s) = default(1s) ; // burst duration time (zero not allowed)
        volatile double sleepDuration @unit(s); // time between bursts (zero allowed)
        
        
        volatile double sendInterval @unit(s); // time between messages during bursts; usually a random value, e.g. 0.1s+uniform(-0.001s,0.001s); zero not allowed
        
        double delayLimit @unit(s) = default(0); // maximum accepted delay for a packet; packets with a bigger delay are discarded (dropped), zero value means no limit
        string outputInterface = default(""); // select the interface in which the packets will be sent
        string outputInterfaceMulticastBroadcast = default(""); // select the interface in which the broadcast packets will be sent
        bool setBroadcast = default(false); // Configure the socket for receive broadcast packets
        bool configureInInit = default(true);

        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
