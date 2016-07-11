/*
  */
#include "BatMobileV1.h"

#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPvXAddressResolver.h"
#include "UDPPacket_m.h"
#include "Ieee802Ctrl_m.h"

Define_Module(BatMobileV1);

std::ostream& operator<<(std::ostream& os, const BatMobileV1_OrigNode& e)
{
    os << e.info();
    return os;
};

std::ostream& operator<<(std::ostream& os, const BatMobileV1_NeighNode& e)
{
    os << e.info();
    return os;
};


BatMobileV1::BatMobileV1() : MobilityAwareRoutingBase()
{
    debug_level = 0;
    debug_level_max = 4;
    gateway_class = 0;
    routing_class = 0;
    originator_interval = 1;
    debug_timeout = 0;
    pref_gateway = ManetAddress::ZERO;

    nat_tool_avail = 0;
    disable_client_nat = 0;

    curr_gateway = NULL;

    found_ifs = 0;
    active_ifs = 0;
    receive_max_sock = 0;

    unix_client = 0;
    log_facility_active = 0;

    origMap.clear();

    if_list.clear();
    gw_list.clear();
    forw_list.clear();
    // struct vis_if vis_if;


    tunnel_running = 0;

    hop_penalty = TQ_HOP_PENALTY;
    purge_timeout = PURGE_TIMEOUT;
    minimum_send = TQ_LOCAL_BIDRECT_SEND_MINIMUM;
    minimum_recv = TQ_LOCAL_BIDRECT_RECV_MINIMUM;
    global_win_size = TQ_GLOBAL_WINDOW_SIZE;
    local_win_size = TQ_LOCAL_WINDOW_SIZE;
    num_words = (TQ_LOCAL_WINDOW_SIZE / WORD_BIT_SIZE);
    aggregation_enabled = true;
    timer = NULL;

    hna_list.clear();
    hna_chg_list.clear();
    hnaMap.clear();
    hna_buff_local.clear();
}

BatMobileV1::~BatMobileV1()
{
    while (!origMap.empty())
    {
        delete origMap.begin()->second;
        origMap.erase(origMap.begin());
    }
    while (!if_list.empty())
    {
        delete if_list.back();
        if_list.pop_back();
    }
    while (!gw_list.empty())
    {
        delete gw_list.back();
        gw_list.pop_back();
    }
    while (!forw_list.empty())
    {
        delete forw_list.back()->pack_buff;
        delete forw_list.back();
        forw_list.pop_back();
    }
    cancelAndDelete(timer);
    while (!hnaMap.empty())
    {
        delete hnaMap.begin()->second;
        hnaMap.erase(hnaMap.begin());
    }
    hna_list.clear();
    hna_buff_local.clear();
    hna_chg_list.clear();
}


void BatMobileV1::initialize(int _stage)
{
    MobilityAwareRoutingBase::initialize(_stage);

    if (_stage == 4)
    {
        // geoassisted
        m_address = getAddress();


        p_prediction->init(p_locationService, 50, 50, 100);
        p_metric->init(p_prediction->getPredictionWidth(), 200);




        //
        found_ifs = 0;

        debug_level = par("debugLevel");
        if (debug_level > debug_level_max) {
            opp_error( "Invalid debug level: %i\nDebug level has to be between 0 and %i.\n", debug_level, debug_level_max );
        }
        purge_timeout = par("purgeTimeout");
        if (purge_timeout <= SIMTIME_ZERO)
            throw cRuntimeError("Invalid 'purgeTimeout' parameter");

        originator_interval = par("originatorInterval");

        if (originator_interval < 0.001)
            throw cRuntimeError("Invalid 'originatorInterval' parameter");

        routing_class = par("routingClass");
        if (routing_class < 0)
            throw cRuntimeError("Invalid 'routingClass' parameter");

        aggregation_enabled = par("aggregationEnable").boolValue();
        disable_client_nat = 1;

        MAX_AGGREGATION_BYTES = par("MAX_AGGREGATION_BYTES");

        int32_t download_speed = 0, upload_speed = 0;

        registerRoutingModule();
        //createTimerQueue();

        const char *preferedGateWay = par("preferedGateWay");
        pref_gateway =  ManetAddress(IPvXAddressResolver().resolve(preferedGateWay, IPvXAddressResolver::ADDR_IPv4));

        /*
        IPv4Address vis = par("visualizationServer");

        if (!vis.isUnspecified())
        {
            vis_server = vis.getInt();
        }
        */

        download_speed = par("GWClass_download_speed");
        upload_speed = par("GWClass_upload_speed");

        if ((download_speed > 0) && (upload_speed == 0))
            upload_speed = download_speed / 5;

        if (download_speed > 0) {
            gateway_class = get_gw_class(download_speed, upload_speed);
            get_gw_speeds(gateway_class, &download_speed, &upload_speed);
        }

        if ((gateway_class != 0) && (routing_class != 0)) {
            opp_error("Error - routing class can't be set while gateway class is in use !\n");
        }

        if ((gateway_class != 0) && (!pref_gateway.isUnspecified())) {
            opp_error("Error - preferred gateway can't be set while gateway class is in use !\n" );
        }

        /* use routing class 1 if none specified */
        if ((routing_class == 0) && (!pref_gateway.isUnspecified()))
            routing_class = DEFAULT_ROUTING_CLASS;

        //if (((routing_class != 0 ) || ( gateway_class != 0 ))&& (!probe_tun(1)))
        //    opp_error("");

        for (int i = 0; i<getNumWlanInterfaces(); i++)
        {
            InterfaceEntry *iEntry = getWlanInterfaceEntry(i);

            BatMobileV1_BatmanIf *batman_if;
            batman_if = new BatMobileV1_BatmanIf();
            batman_if->dev = iEntry;
            batman_if->if_num = found_ifs;
            batman_if->seqno = 1;

            batman_if->wifi_if = true;
            batman_if->if_active = true;
            if (isInMacLayer())
            {
                batman_if->address = ManetAddress(iEntry->getMacAddress());
                batman_if->broad = ManetAddress(MACAddress::BROADCAST_ADDRESS);
            }
            else
            {
                batman_if->address = ManetAddress(iEntry->ipv4Data()->getIPAddress());
                batman_if->broad = ManetAddress(IPv4Address::ALLONES_ADDRESS);
            }

            batman_if->if_rp_filter_old = -1;
            batman_if->if_send_redirects_old = -1;
            if_list.push_back(batman_if);
            if (batman_if->if_num > 0)
                hna_local_task_add_ip(batman_if->address, 32, ROUTE_ADD);  // XXX why is it sending an HNA record at all? HNA should be sent only for networks
            found_ifs++;
        }
        log_facility_active = 1;

        // parse announcedNetworks parameter
        const char *announcedNetworks = par("announcedNetworks");
        cStringTokenizer tokenizer(announcedNetworks);
        const char *token;
        while ((token = tokenizer.nextToken()) != NULL)
        {
            std::vector<std::string> addrPair = cStringTokenizer(token, "/").asVector();
            if (addrPair.size() != 2)
                throw cRuntimeError("invalid 'announcedNetworks' parameter content: '%s'", token);

            IPv4Address addr = IPvXAddressResolver().resolve(addrPair[0].c_str()).get4();
            IPv4Address mask = IPvXAddressResolver().resolve(addrPair[1].c_str(), IPvXAddressResolver::ADDR_MASK).get4();
            addr.doAnd(mask);

            // add to HNA:
            hna_local_task_add_ip(ManetAddress(addr), mask.getNetmaskLength(), ROUTE_ADD);
        }

        /* add rule for hna networks */
        //add_del_rule(0, 0, BATMAN_RT_TABLE_NETWORKS, BATMAN_RT_PRIO_UNREACH - 1, 0, RULE_TYPE_DST, RULE_ADD);

        /* add unreachable routing table entry */
        //add_del_route(0, 0, 0, 0, 0, "unknown", BATMAN_RT_TABLE_UNREACH, ROUTE_TYPE_UNREACHABLE, ROUTE_ADD);

        if (routing_class > 0) {
            if (add_del_interface_rules(RULE_ADD) < 0) {
                opp_error("BATMAN Interface error");
            }
        }

        //if (gateway_class != 0)
        //    init_interface_gw();

        for (unsigned int i = 0; i < if_list.size(); i++)
        {
            BatMobileV1_BatmanIf *batman_if = if_list[i];
            schedule_own_packet(batman_if);
        }

        timer = new cMessage();
        WATCH_PTRMAP(origMap);

        simtime_t curr_time = simTime();
        simtime_t select_timeout = forw_list[0]->send_time > curr_time ? forw_list[0]->send_time : curr_time+10;
        scheduleAt(select_timeout, timer);
    }



}


void BatMobileV1::handleMessage(cMessage *msg)
{
    BatMobileV1_BatmanIf *if_incoming;
    ManetAddress neigh;
    simtime_t vis_timeout, select_timeout, curr_time;

    if (!isNodeOperational())
    {
        delete msg;
        return;
    }

    curr_time = getTime();
    check_active_inactive_interfaces();
    if (timer == msg)
    {
        sendPackets(curr_time);
        return;
    }

    /* harden select_timeout against sudden time change (e.g. ntpdate) */
    //select_timeout = ((int)(((struct forw_node *)forw_list.next)->send_time - curr_time) > 0 ?
    //            ((struct forw_node *)forw_list.next)->send_time - curr_time : 10);

    if (!this->isInMacLayer())
    {
        IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(msg->removeControlInfo());
        IPvXAddress srcAddr = ctrl->getSrcAddr();
        neigh = ManetAddress(srcAddr);
        for (unsigned int i=0; i<if_list.size(); i++)
        {
            if (if_list[i]->dev->getInterfaceId() == ctrl->getInterfaceId())
            {
                if_incoming = if_list[i];
                break;
            }
        }
        delete ctrl;
    }
    else
    {
        Ieee802Ctrl* ctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
        MACAddress srcAddr = ctrl->getSrc();
        neigh = ManetAddress(srcAddr);
        if_incoming = if_list[0];
        delete ctrl;
    }


    BatMobileV1_BatmanPacket *bat_packet = NULL;
    UDPPacket *udpPacket = dynamic_cast<UDPPacket*>(msg);
    if (udpPacket)
    {
        if (udpPacket->getDestinationPort() != BATMAN_PORT)
        {
            delete  msg;
            sendPackets(curr_time);
            return;
        }

        cMessage* msg_aux = udpPacket->decapsulate();
        bat_packet = dynamic_cast <BatMobileV1_BatmanPacket*>(msg_aux);
        if (!bat_packet)
        {
            delete msg;
            delete msg_aux;
            sendPackets(curr_time);
            return;
        }

        delete msg;
    }
    else if (this->isInMacLayer())
    {
        if (!dynamic_cast<BatMobileV1_BatmanPacket*>(msg))
        {
            delete  msg;
            sendPackets(curr_time);
            return;
        }
    }

    parseIncomingPacket(neigh, if_incoming, bat_packet);

    sendPackets(curr_time);
}

void BatMobileV1::sendPackets(const simtime_t &curr_time)
{
    send_outstanding_packets(curr_time);
    if (curr_time - debug_timeout > 1) {

        debug_timeout = curr_time;
        purge_orig( curr_time );
        //check_inactive_interfaces();
        if ( ( routing_class != 0 ) && ( curr_gateway == NULL ) )
            choose_gw();
        hna_local_task_exec();
    }
    scheduleNextEvent();
}

void BatMobileV1::scheduleNextEvent()
{
     simtime_t select_timeout = forw_list[0]->send_time > 0 ? forw_list[0]->send_time : getTime()+10;
     if (timer->isScheduled())
     {
         if (timer->getArrivalTime()>select_timeout)
         {
             cancelEvent(timer);
             if (select_timeout>simTime())
                 scheduleAt(select_timeout, timer);
             else
                 scheduleAt(simTime(), timer);
         }
     }
     else
     {
         if (select_timeout>simTime())
             scheduleAt(select_timeout, timer);
         else
             scheduleAt(simTime(), timer);
     }
}

uint32_t BatMobileV1::getRoute(const ManetAddress &dest, std::vector<ManetAddress> &add)
{
    BatMobileV1_OrigMap::iterator it = origMap.find(dest);
    if (it != origMap.end())
    {
        BatMobileV1_OrigNode *node = it->second;
        add.resize(0);
        add.push_back(node->router->addr);
        return -1;
    }
    ManetAddress apAddr;
    if (getAp(dest,apAddr))
    {
        BatMobileV1_OrigMap::iterator it = origMap.find(apAddr);
        if (it != origMap.end())
        {
            BatMobileV1_OrigNode *node = it->second;
            add.resize(0);
            add.push_back(node->router->addr);
            return -1;
        }
    }
    return 0;
}

bool BatMobileV1::getNextHop(const ManetAddress &dest, ManetAddress &add, int &iface, double &val)
{
    BatMobileV1_OrigMap::iterator it = origMap.find(dest);
    if (it != origMap.end())
    {
        BatMobileV1_OrigNode *node = it->second;
        add = node->router->addr;
        return true;
    }
    ManetAddress apAddr;
    if (getAp(dest,apAddr))
    {
        BatMobileV1_OrigMap::iterator it = origMap.find(apAddr);
        if (it != origMap.end())
        {
            BatMobileV1_OrigNode *node = it->second;
            add = node->router->addr;
            return true;
        }
    }
    return false;
}

void BatMobileV1::appendPacket(cPacket *oldPacket, cPacket *packetToAppend)
{
    if (oldPacket->getEncapsulatedPacket()==NULL)
    {
        oldPacket->encapsulate(packetToAppend);
        return;
    }
    std::vector<cPacket*> vectorPacket;
    cPacket *pkt = oldPacket;
    while (pkt->getEncapsulatedPacket())
    {
        vectorPacket.push_back(pkt->decapsulate());
        pkt = vectorPacket.back();
    }
    pkt = packetToAppend;
    while (!vectorPacket.empty())
    {
          cPacket *pktAux = vectorPacket.back();
          pktAux->encapsulate(pkt);
          pkt = pktAux;
          vectorPacket.pop_back();
    }
    oldPacket->encapsulate(pkt);
}

BatMobileV1_BatmanPacket *BatMobileV1::buildDefaultBatmanPkt(const BatMobileV1_BatmanIf *batman_if)
{
    m_nodeMap.update();
    updateRoutingTable(); // triggered

    std::string str = "BatmanPkt:" + batman_if->address.str();
    BatMobileV1_BatmanPacket *pkt = new BatMobileV1_BatmanPacket(str.c_str());

    pkt->setVersion(0);
    pkt->setFlags(0x00);
    pkt->setTtl((batman_if->if_num > 0 ? 2 : TTL));
    pkt->setGatewayFlags(batman_if->if_num > 0 ? 0 : gateway_class);
    pkt->setSeqNumber(batman_if->seqno);
    pkt->setGatewayPort(GW_PORT);
    pkt->setTq(TQ_MAX_VALUE);
    if (batman_if->if_active)
    {
       pkt->setOrig(batman_if->address);
       pkt->setPrevSender(batman_if->address);
    }

    if(p_locationService && p_prediction)
    {
        Coord currentPosition = p_locationService->getCurrentMobilityData().position;
        Coord predictedPosition = p_prediction->predictOwnData(simTime().dbl() * 1000).position;
        double score = 1;

        pkt->setPosition(currentPosition);
        pkt->setPredictedPosition(predictedPosition);
        pkt->setScore(score);
    }

    return pkt;
}


bool BatMobileV1::handleNodeStart(IDoneCallback *doneCallback)
{
    for (unsigned int i = 0; i < if_list.size(); i++)
    {
        BatMobileV1_BatmanIf *batman_if = if_list[i];
        schedule_own_packet(batman_if);
    }

    simtime_t curr_time = simTime();
    simtime_t select_timeout = forw_list[0]->send_time > curr_time ? forw_list[0]->send_time : curr_time+10;
    scheduleAt(select_timeout, timer);
    scheduleNextEvent();
    return true;
}

bool BatMobileV1::handleNodeShutdown(IDoneCallback *doneCallback)
{

    while (!origMap.empty())
    {
        delete origMap.begin()->second;
        origMap.erase(origMap.begin());
    }
    while (!if_list.empty())
    {
        delete if_list.back();
        if_list.pop_back();
    }
    while (!gw_list.empty())
    {
        delete gw_list.back();
        gw_list.pop_back();
    }
    while (!forw_list.empty())
    {
        delete forw_list.back()->pack_buff;
        delete forw_list.back();
        forw_list.pop_back();
    }
    cancelEvent(timer);
    while (!hnaMap.empty())
    {
        delete hnaMap.begin()->second;
        hnaMap.erase(hnaMap.begin());
    }
    hna_list.clear();
    hna_buff_local.clear();
    hna_chg_list.clear();
    return true;
}

void BatMobileV1::handleNodeCrash()
{

    while (!origMap.empty())
    {
        delete origMap.begin()->second;
        origMap.erase(origMap.begin());
    }
    while (!if_list.empty())
    {
        delete if_list.back();
        if_list.pop_back();
    }
    while (!gw_list.empty())
    {
        delete gw_list.back();
        gw_list.pop_back();
    }
    while (!forw_list.empty())
    {
        delete forw_list.back()->pack_buff;
        delete forw_list.back();
        forw_list.pop_back();
    }
    cancelEvent(timer);
    while (!hnaMap.empty())
    {
        delete hnaMap.begin()->second;
        hnaMap.erase(hnaMap.begin());
    }
    hna_list.clear();
    hna_buff_local.clear();
    hna_chg_list.clear();
}
