/*
 * Copyright (C) 2006-2009 B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich, Axel Neumann, Marek Lindner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include "BatMobileV1_batman.h"
#include "BatMobileV1.h"
#include "IPv4InterfaceData.h"


int8_t BatMobileV1::send_udp_packet(cPacket *packet_buff, int32_t packet_buff_len, const ManetAddress & destAdd, int32_t send_sock, BatMobileV1_BatmanIf *batman_if)
{
    if ((batman_if != NULL) && (!batman_if->if_active))
    {
        delete packet_buff;
        return 0;
    }
    if (batman_if)
        sendToIp(packet_buff, BATMAN_PORT, destAdd, BATMAN_PORT, 1, par("broadcastDelay").doubleValue(), ManetAddress(batman_if->dev->ipv4Data()->getIPAddress()));
    else
        sendToIp(packet_buff, BATMAN_PORT, destAdd, BATMAN_PORT, 1, par("broadcastDelay").doubleValue(), ManetAddress::ZERO);
    return 0;
}

//
//
// modification routing tables methods
//
//
void BatMobileV1::add_del_route(const ManetAddress &dest, uint8_t netmask, const ManetAddress &router, int32_t ifi, InterfaceEntry* dev, uint8_t rt_table, int8_t route_type, int8_t route_action)
{
    if (route_type != ROUTE_TYPE_UNICAST)
        return;
    int index = -1;
    for (int i=0; i<getNumInterfaces(); i++)
    {
        if (dev == this->getInterfaceEntry(i))
        {
            index = i;
            break;
        }
    }
    if (index < 0)
        return;

    ManetAddress nmask(IPv4Address::makeNetmask(netmask));
    if (route_action==ROUTE_DEL)
    {
       setRoute(dest, ManetAddress::ZERO, index, 0, nmask);
       return;
    }

    // if (route_action==ROUTE_ADD)
    setRoute(dest, router, index, -1, nmask);
}

int BatMobileV1::add_del_interface_rules(int8_t rule_action)
{
    if (isInMacLayer())
        return 1;
    int if_count = 1;
    for (int i=0; i<getNumInterfaces(); i++)
    {
        InterfaceEntry *ifr = getInterfaceEntry(i);

        if (ifr->ipv4Data()==NULL) // no ipv4
            continue;
        if (ifr->isLoopback())      //FIXME What would be the correct conditions here? The isLoopback() used below.
            continue;
        if (!ifr->isUp())
            continue;

        ManetAddress addr(ifr->ipv4Data()->getIPAddress());
        ManetAddress netmask(ifr->ipv4Data()->getNetmask());
        uint8_t mask = ifr->ipv4Data()->getNetmask().getNetmaskLength();

        ManetNetworkAddress netaddr(addr, mask);// addr&netmask;
        BatMobileV1_BatmanIf *batman_if;

        ManetAddress ZERO;
        add_del_route(netaddr.getAddress(), mask, ZERO, 0, ifr, BATMAN_RT_TABLE_TUNNEL, ROUTE_TYPE_THROW, rule_action);

        if ((batman_if = is_batman_if(ifr))==NULL)
            continue;

        add_del_rule(netaddr.getAddress(), mask, BATMAN_RT_TABLE_TUNNEL, (rule_action == RULE_DEL ? 0 : BATMAN_RT_PRIO_TUNNEL + if_count), 0, RULE_TYPE_SRC, rule_action);

        if (ifr->isLoopback())
            add_del_rule(ManetAddress::ZERO, 0, BATMAN_RT_TABLE_TUNNEL, BATMAN_RT_PRIO_TUNNEL, ifr, RULE_TYPE_IIF, rule_action);
        if_count++;
    }

    return 1;
}

void BatMobileV1::add_del_rule(const ManetAddress& network, uint8_t netmask, int8_t rt_table, uint32_t prio, InterfaceEntry *iif, int8_t rule_type, int8_t rule_action)
{
    return;
}

void BatMobileV1::deactivate_interface(BatMobileV1_BatmanIf *iface)
{
    iface->if_active = false;
    //iface->dev->setDown(true);
}

void BatMobileV1::activate_interface(BatMobileV1_BatmanIf *iface)
{
    iface->if_active = true;

    //iface->dev->setDown(true);
}

void BatMobileV1::check_active_inactive_interfaces(void)
{
    /* all available interfaces are deactive */
    for (unsigned int i=0; i<if_list.size(); i++){
        BatMobileV1_BatmanIf* batman_if = if_list[i];
        if ((batman_if->if_active) && (!batman_if->dev->isUp()))
        {
            deactivate_interface(batman_if);
            active_ifs--;
        }
        else if ((!batman_if->if_active) && (batman_if->dev->isUp()))
        {
            activate_interface(batman_if);
            active_ifs++;
        }
    }
}


void BatMobileV1::check_inactive_interfaces(void)
{
    /* all available interfaces are active */
    if (found_ifs == active_ifs)
        return;

    for (unsigned int i=0; i<if_list.size(); i++){
        BatMobileV1_BatmanIf* batman_if = if_list[i];

        if ((!batman_if->if_active) && (batman_if->dev->isUp()))
        {
            activate_interface(batman_if);
            active_ifs++;
        }
    }
}

void BatMobileV1::check_active_interfaces(void)
{
    /* all available interfaces are deactive */
    if (active_ifs == 0)
        return;
    for (unsigned int i=0; i<if_list.size(); i++)
    {
        BatMobileV1_BatmanIf* batman_if = if_list[i];
        if ((batman_if->if_active) && (!batman_if->dev->isUp()))
        {
            deactivate_interface(batman_if);
            active_ifs--;
        }
    }
}

simtime_t BatMobileV1::getTime()
{
    return simTime() + par("desynchronized");
}

