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

#include "BatMobile.h"
#include "UDPPacket_m.h"

#include "IPv4ControlInfo.h"

Define_Module(BatMobile);

BatMobile::BatMobile() : MobilityAwareRoutingBase(),
        m_updateMessage(0),
        m_sequenceNumber(0)
{

}

BatMobile::~BatMobile()
{
    if(m_updateMessage)
        cancelAndDelete(m_updateMessage);
}

void BatMobile::broadcastBatMobilePacket()
{
    std::string packetName = "BatMobilePacket:" + m_address.str();
    BatMobilePacket *packet = new BatMobilePacket(packetName.c_str());

    if(p_locationService)
    {
        Coord currentPosition = p_locationService->getCurrentMobilityData().position;
        Coord predictedPosition = p_prediction->predictOwnData(simTime().dbl() * 1000).position;
        double score = 1;

        packet->setSourceAddress(m_address);
        packet->setSequenceNumber(updateSequenceNumber());
        packet->setTtl(BATMOBILE_TTL);

        packet->setPosition(currentPosition);
        packet->setPredictedPosition(predictedPosition);
        packet->setScore(score);

        forwardBatMobilePacket(packet);
    }
}

void BatMobile::forwardBatMobilePacket(BatMobilePacket *_packet)
{
    broadcastPacket(_packet, BATMOBILE_PORT, 1); // the ttl is handled by BatMobile itself
}

void BatMobile::handleBatMobilePacket(BatMobilePacket *_packet, const ManetAddress &_forwarder)
{
    if(p_locationService)
    {
        // handle routing specific data
        ManetAddress sourceAddress = _packet->getSourceAddress();
        int ttl = _packet->getTtl();
        int sequenceNumber = _packet->getSequenceNumber();


        // compute the new path score
        Coord currentPosition = p_locationService->getCurrentMobilityData().position;
        Coord predictedPosition = p_prediction->predictOwnData(simTime().dbl() * 1000).position;

        Coord neighborPosition = _packet->getPosition();
        Coord predictedNeighborPosition = _packet->getPredictedPosition();

        double currentDistance_m = currentPosition.distance(_packet->getPosition());
        double predictedDistance_m = predictedPosition.distance(_packet->getPredictedPosition());
        double stepWidth_m = p_mobility->getStepWidth();
        double score = p_metric->updatePathScore(_packet->getScore(), currentDistance_m, predictedDistance_m, stepWidth_m, stepWidth_m);


        // update the neighbor ranking for the destination
        bool sequenceNumberUpdated = true;
        NodeEntry *destinationNode = m_nodeMap.getDestinationNodeEntry(sourceAddress);
        if(!destinationNode)
            destinationNode = m_nodeMap.createNodeEntry(sourceAddress, sequenceNumber);
        else
            sequenceNumberUpdated = destinationNode->updateSequenceNumber(sequenceNumber);
        destinationNode->updateNeighborScore(_forwarder, score);


        // update the neighbor ranking for the forwarder
        double forwarderScore = p_metric->updatePathScore(1, currentDistance_m, predictedDistance_m, stepWidth_m, stepWidth_m);
        NodeEntry *forwarderNode = m_nodeMap.getDestinationNodeEntry(_forwarder);
        if(!forwarderNode)
            forwarderNode = m_nodeMap.createNodeEntry(sourceAddress, 0);
        forwarderNode->updateNeighborScore(_forwarder, forwarderScore);


        // forward the packet if the ttl is high enough
        if(sequenceNumberUpdated && ttl>0)
        {
            _packet->setTtl(ttl-1);
            _packet->setPosition(currentPosition);
            _packet->setPredictedPosition(predictedPosition);
            _packet->setScore(score);

            forwardBatMobilePacket(_packet);
        }
        else
            delete _packet;
    }
}

void BatMobile::updateRoutingTable()
{
    // for all destinations
    std::vector<NodeEntry*> entries = m_nodeMap.getEntries();
    for(unsigned int i=0; i<entries.size(); i++)
    {
        // find the best neighbor node
        NodeEntry *destination = entries.at(i);
        NeighborNodeEntry *neighbor = destination->getBestNeighbor();

        updateRoute(destination, neighbor, 2);
    }
}

unsigned short BatMobile::updateSequenceNumber()
{
    if(m_sequenceNumber==0xffff)
        m_sequenceNumber = 0;

    m_sequenceNumber++;

    return m_sequenceNumber;
}

void BatMobile::debugPacket(BatMobilePacket *_packet, const ManetAddress &_forwarder)
{
    std::cout << "********************" << m_address.str() << "********************" << std::endl;
    std::cout << "from: " << _packet->getSourceAddress().str() << "\tvia: " << _forwarder.str() << std::endl;
    std::cout << "score: " << _packet->getScore() <<"\t\tseq: " << _packet->getSequenceNumber() << "\t\tttl: " << (int)_packet->getTtl() << std::endl;;
}

void BatMobile::handleMessage(cMessage *_msg)
{
    if(_msg->isSelfMessage())
    {
        m_nodeMap.update();
        broadcastBatMobilePacket();
        updateRoutingTable();

        scheduleAt(simTime()+m_broadcastInterval_s + par("broadcastJitter"), m_updateMessage);
    }
    else
    {
        ManetAddress forwarder;
        if (!this->isInMacLayer())
        {
            IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(_msg->removeControlInfo());
            forwarder = ManetAddress(ctrl->getSrcAddr());

            delete ctrl;
        }

        UDPPacket *udpPacket = dynamic_cast<UDPPacket*>(_msg);
        if (udpPacket)
        {
            if (udpPacket->getDestinationPort() != BATMOBILE_PORT)
            {
                delete _msg;
                return;
            }

            cMessage* msg_aux = udpPacket->decapsulate();

            BatMobilePacket *batmobilePacket = dynamic_cast <BatMobilePacket*>(msg_aux);
            if (!batmobilePacket)
            {
                delete _msg;
                delete msg_aux;
                return;
            }
            else
            {
                handleBatMobilePacket(batmobilePacket->dup(), forwarder);

                delete _msg;
                delete msg_aux;
                return;
            }

        }
        else
        {
            delete _msg;
            return;
        }
    }
}

void BatMobile::initialize(int _stage)
{
    MobilityAwareRoutingBase::initialize(_stage);

    if(_stage==4)
    {
        registerRoutingModule();

        m_address = getAddress();
        m_broadcastInterval_s = par("broadcastInterval").doubleValue();

        p_prediction->init(p_locationService, 50, 50, 100);
        p_metric->init(p_prediction->getPredictionWidth(), 200);


        // start the message broadcast process
        m_updateMessage = new cMessage("update");
        scheduleAt(simTime() + m_broadcastInterval_s + par("broadcastJitter"), m_updateMessage);
    }
}
