/*
 * Copyright (C) 2006-2009 B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich, Marek Lindner
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


#include "BatMobileV1.h"



/* this function can be called when the daemon starts or at runtime */
void BatMobileV1::hna_local_task_add_ip(const ManetAddress &ip_addr, uint16_t netmask, uint8_t route_action)
{
    BatMobileV1_HnaTask hna_task;

    hna_task.addr = ip_addr;
    hna_task.netmask = netmask;
    hna_task.route_action = route_action;

    hna_chg_list.push_back(hna_task);
}
void BatMobileV1::hna_local_buffer_fill(void)
{
    hna_buff_local.clear();

    if (hna_list.empty())
        return;

    for (unsigned int list_pos = 0; list_pos < hna_list.size(); list_pos++) {
        BatMobileV1_HnaElement aux;
        aux.addr = hna_list[list_pos].addr;
        aux.netmask = hna_list[list_pos].netmask;
        hna_buff_local.push_back(aux);
    }
}

void BatMobileV1::hna_local_task_exec(void)
{
    BatMobileV1_HnaTask *hna_task;
    BatMobileV1_HnaLocalEntry *hna_local_entry;

    if (hna_chg_list.empty())
        return;

    while (!hna_chg_list.empty()) {
        hna_task = &hna_chg_list.front();

        hna_local_entry = NULL;
        bool found = false;

        for (unsigned int hna_pos = 0; hna_pos < hna_list.size(); hna_pos++) {
            hna_local_entry = &(hna_list[hna_pos]);

            if ((hna_task->addr == hna_local_entry->addr) && (hna_task->netmask == hna_local_entry->netmask)) {
                found = true;
                if (hna_task->route_action == ROUTE_DEL) {
                    debug_output(3) << "Deleting HNA from announce network list: " << hna_task->addr << "/" << hna_task->netmask << endl;

                    hna_local_update_routes(hna_local_entry, ROUTE_DEL);
                    hna_list.erase(hna_list.begin()+hna_pos);
                    --hna_pos;
                } else {
                    debug_output(3) << "Can't add HNA - already announcing network: " << hna_task->addr << "/" << hna_task->netmask << endl;
                }
                break;
            }
        }

        if (!found) {
            if (hna_task->route_action == ROUTE_ADD) {
                debug_output(3) << "Adding HNA to announce network list: " << hna_task->addr << "/" << hna_task->netmask << endl;

                /* add node */
                BatMobileV1_HnaLocalEntry hna_local_entry;

                hna_local_entry.addr = hna_task->addr;
                hna_local_entry.netmask = hna_task->netmask;

                hna_local_update_routes(&hna_local_entry, ROUTE_ADD);
                hna_list.push_back(hna_local_entry);
            } else {
                debug_output(3) << "Can't delete HNA - network is not announced: " << hna_task->addr << "/" << hna_task->netmask << endl;
            }
        }

        hna_chg_list.erase(hna_chg_list.begin());
    }

    /* rewrite local buffer */
    hna_local_buffer_fill();
}

void BatMobileV1::hna_local_update_routes(BatMobileV1_HnaLocalEntry *hna_local_entry, int8_t route_action)
{
    /* add / delete throw routing entries for own hna */
    //add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_THROW, route_action);
    //add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_THROW, route_action);
    //add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_UNREACH, ROUTE_TYPE_THROW, route_action);
    //add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_TUNNEL, ROUTE_TYPE_THROW, route_action);
    add_del_route(hna_local_entry->addr, hna_local_entry->netmask, ManetAddress::ZERO, 0, NULL, BATMAN_RT_TABLE_TUNNEL, ROUTE_TYPE_THROW, route_action);

    /* do not NAT HNA networks automatically */
    //hna_local_update_nat(hna_local_entry->addr, hna_local_entry->netmask, route_action);
}

void BatMobileV1::_hna_global_add(BatMobileV1_OrigNode *orig_node, BatMobileV1_HnaElement *hna_element)
{
    HnaGlobalEntry *hna_global_entry;
    BatMobileV1_OrigNode *old_orig_node = NULL;

    std::map<BatMobileV1_HnaElement,HnaGlobalEntry*>::iterator it;
    it = hnaMap.find(*hna_element);

    /* add the hna node if it does not exist */
    if (it == hnaMap.end())
    {
        hna_global_entry = new HnaGlobalEntry();
        hna_global_entry->addr = hna_element->addr;
        hna_global_entry->netmask = hna_element->netmask;
        hna_global_entry->curr_orig_node = NULL;
        hnaMap.insert(std::pair<BatMobileV1_HnaElement,HnaGlobalEntry*>(*hna_element, hna_global_entry));
    }
    else
        hna_global_entry = it->second;

    /* the given orig_node already is the current orig node for this HNA */
    if (hna_global_entry->curr_orig_node == orig_node)
        return;

    bool notFound = true;
    for (unsigned int list_pos = 0; list_pos < hna_global_entry->orig_list.size(); list_pos++) {
        if (hna_global_entry->orig_list[list_pos] == orig_node) {
            notFound = false;
            break;
        }
    }

    /* append the given orig node to the list */
    if (notFound)
        hna_global_entry->orig_list.push_back(orig_node);

    /* our TQ value towards the HNA is better */
    if ((!hna_global_entry->curr_orig_node) ||
        (orig_node->router->tq_avg > hna_global_entry->curr_orig_node->router->tq_avg)) {
        old_orig_node = hna_global_entry->curr_orig_node;
        hna_global_entry->curr_orig_node = orig_node;

        /**
         * if we change the orig node towards the HNA we may still route via the same next hop
         * which does not require any routing table changes
         */
        if ((old_orig_node) &&
            (hna_global_entry->curr_orig_node->router->addr == old_orig_node->router->addr))
            return;

        /* delete previous route */
        // should delete before add, see ManetRoutingBase::setRoute()
        if (old_orig_node) {
            add_del_route(hna_element->addr, hna_element->netmask, old_orig_node->router->addr,
                        old_orig_node->router->if_incoming->if_index,
                        old_orig_node->router->if_incoming->dev,
                        BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);
        }

        add_del_route(hna_element->addr, hna_element->netmask, orig_node->router->addr,
                    orig_node->router->if_incoming->if_index,
                    orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);
    }

}

void BatMobileV1::_hna_global_del(BatMobileV1_OrigNode *orig_node, BatMobileV1_HnaElement *hna_element)
{
    HnaGlobalEntry *hna_global_entry;
    BatMobileV1_OrigNode *orig_ptr = NULL;

    std::map<BatMobileV1_HnaElement,HnaGlobalEntry*>::iterator it;
    it = hnaMap.find(*hna_element);

    /* add the hna node if it does not exist */
    if (it == hnaMap.end())
        return;

    hna_global_entry = it->second;
    hna_global_entry->curr_orig_node = NULL;

    for (unsigned int i=0; i<hna_global_entry->orig_list.size(); ) {
        orig_ptr = hna_global_entry->orig_list[i];

        /* delete old entry in orig list */
        if (orig_ptr == orig_node) {
            hna_global_entry->orig_list.erase(hna_global_entry->orig_list.begin()+i);
            continue;
        }

        /* find best alternative route */
        if ((!hna_global_entry->curr_orig_node) ||
            (orig_ptr->router->tq_avg > hna_global_entry->curr_orig_node->router->tq_avg))
            hna_global_entry->curr_orig_node = hna_global_entry->orig_list[i];
        i++;
    }

    /* set new route if available */
    if (hna_global_entry->curr_orig_node) {
        /**
         * if we delete one orig node towards the HNA but we switch to an alternative
         * which is reachable via the same next hop no routing table changes are necessary
         */
        if (hna_global_entry->curr_orig_node->router->addr == orig_node->router->addr)
            return;
    }

    // should delete before add, see ManetRoutingBase::setRoute()
    add_del_route(hna_element->addr, hna_element->netmask, orig_node->router->addr,
                orig_node->router->if_incoming->if_index,
                orig_node->router->if_incoming->dev,
                BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);

    if (hna_global_entry->curr_orig_node) {
        add_del_route(hna_element->addr, hna_element->netmask, hna_global_entry->curr_orig_node->router->addr,
                    hna_global_entry->curr_orig_node->router->if_incoming->if_index,
                    hna_global_entry->curr_orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);
    }

    /* if no alternative route is available remove the HNA entry completely */
    if (!hna_global_entry->curr_orig_node) {
        hnaMap.erase(it);
        delete hna_global_entry;
    }
}

/* hna_buff_delete searches in buf if element e is found.
 *
 * if found, delete it from the buf and return 1.
 * if not found, return 0.
 */
int BatMobileV1::hna_buff_delete(std::vector<BatMobileV1_HnaElement> &buf, int *buf_len, BatMobileV1_HnaElement *e)
{
    for (unsigned int i = 0; i < buf.size(); i++) {
        if ((buf[i]) == *e) {
            /* move last element forward */
            buf.erase(buf.begin()+i);
            *buf_len -= SIZE_Hna_element;

            return 1;
        }
    }
    return 0;
}

void BatMobileV1::hna_global_add(BatMobileV1_OrigNode *orig_node, BatMobileV1_HnaElement *new_hna, int16_t new_hna_len)
{
    BatMobileV1_HnaElement *e;
    int i, num_elements;
    // clean buffer
    orig_node->hna_buff.clear();

    if ((new_hna == NULL) || (new_hna_len == 0)) {
        return;
    }

    /* add new routes */
    num_elements = new_hna_len / SIZE_Hna_element;

    debug_output(4) << "HNA information received (" << num_elements << " HNA network" << (num_elements > 1 ? "s": "") << "):\n";

    for (i = 0; i < num_elements; i++) {
        e = &new_hna[i];
        orig_node->hna_buff.push_back(*e);
        //addr_to_string(e->addr, hna_str, sizeof(hna_str));

        if ((e->netmask > 0) && (e->netmask < 33))
            debug_output(4) << "hna: " << e->addr << "/" << (unsigned int)(e->netmask) << "\n";
        else
            debug_output(4) << "hna: " << e->addr << "/" << (unsigned int)(e->netmask) << " -> ignoring (invalid netmask) \n";

        if ((e->netmask > 0) && (e->netmask <= 32))
            _hna_global_add(orig_node, e);
    }
}

/**
 * hna_global_update() replaces the old add_del_hna function. This function
 * updates the new hna buffer for the supplied orig node and
 * adds/deletes/updates the announced routes.
 *
 * Instead of first deleting and then adding, we try to add new routes
 * before delting the old ones so that the kernel will not experience
 * a situation where no route is present.
 */
void BatMobileV1::hna_global_update(BatMobileV1_OrigNode *orig_node, BatMobileV1_HnaElement *new_hna, int16_t new_hna_len, BatMobileV1_NeighNode *old_router)
{
    BatMobileV1_HnaElement *e;
    HnaGlobalEntry *hna_global_entry;
    int i, num_elements, old_hna_len;
    std::vector<BatMobileV1_HnaElement> old_hna;

    /* orig node stopped announcing any networks */
    if ((!orig_node->hna_buff.empty()) && ((new_hna == NULL) || (new_hna_len == 0))) {
        hna_global_del(orig_node);
        return;
    }

    /* orig node started to announce networks */
    if ((orig_node->hna_buff.empty()) && ((new_hna != NULL) || (new_hna_len != 0))) {
        hna_global_add(orig_node, new_hna, new_hna_len);
        return;
    }

    /**
     * next hop router changed - no need to change the global hna hash
     * we just have to make sure that the best orig node is still in place
     * NOTE: we may miss a changed HNA here which we will update with the next packet
     */
    if (old_router != orig_node->router) {
        num_elements = orig_node->hna_buff.size();

        for (i = 0; i < num_elements; i++) {
            e = &orig_node->hna_buff[i];

            if ((e->netmask < 1) || (e->netmask > 32))
                continue;

            std::map<BatMobileV1_HnaElement,HnaGlobalEntry*>::iterator it;
            it = hnaMap.find(*e);

            /* add the hna node if it does not exist */
            if (it == hnaMap.end())
                return;
            hna_global_entry = it->second;

            if (!hna_global_entry)
                continue;

            /* if the given orig node is not in use no routes need to change */
            if (hna_global_entry->curr_orig_node != orig_node)
                continue;

            // should delete before add, see ManetRoutingBase::setRoute()
            add_del_route(e->addr, e->netmask, old_router->addr,
                    old_router->if_incoming->if_index,
                    old_router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);

            add_del_route(e->addr, e->netmask, orig_node->router->addr,
                    orig_node->router->if_incoming->if_index,
                    orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);
        }

        return;
    }

    /**
     * check if the buffers even changed. if its still the same, there is no need to
     * update the routes. if the router changed, then we have to update all the routes
     * NOTE: no NULL pointer checking here because memcmp() just returns if n == 0
     */
    bool change = false;
    if ((int)orig_node->hna_buff.size() * SIZE_Hna_element != new_hna_len)
        change = true;
    else
    {
        for (unsigned int i=0; i<orig_node->hna_buff.size(); i++)
        {
            if ((orig_node->hna_buff[i]) != new_hna[i])
            {
                change = true;
                break;
            }
        }
    }
    if (!change)
        return;    /* nothing to do */

    /* changed HNA */
    old_hna.swap(orig_node->hna_buff);

    orig_node->hna_buff.clear();
    num_elements = new_hna_len / SIZE_Hna_element;
    for (i = 0; i < num_elements; i++) {
        e = &new_hna[i];
        orig_node->hna_buff.push_back(*e);
    }

    /* add new routes and keep old routes */
    for (i = 0; i < num_elements; i++) {
        e = &orig_node->hna_buff[i];

        /**
         * if the router is the same, and the announcement was already in the old
         * buffer, we can keep the route.
         */
        if (hna_buff_delete(old_hna, &old_hna_len, e) == 0) {
            /* not found / deleted, need to add this new route */
            if ((e->netmask > 0) && (e->netmask <= 32))
                _hna_global_add(orig_node, e);
        }
    }

    /* old routes which are not to be kept are deleted now. */
    num_elements = old_hna_len / SIZE_Hna_element;

    for (unsigned int i = 0; i < old_hna.size(); i++) {
        e = &old_hna[i];

        if ((e->netmask > 0) && (e->netmask <= 32))
            _hna_global_del(orig_node, e);
    }

    /* dispose old hna buffer now. */
    old_hna.clear();
}

void BatMobileV1::hna_global_check_tq(BatMobileV1_OrigNode *orig_node)
{
    BatMobileV1_HnaElement *e;
    HnaGlobalEntry *hna_global_entry;
    int i, num_elements;

    if (orig_node->hna_buff.empty())
        return;

    num_elements = orig_node->hna_buff.size();
    for (i = 0; i < num_elements; i++) {
        e = &orig_node->hna_buff[i];

        if ((e->netmask < 1) || (e->netmask > 32))
            continue;

        std::map<BatMobileV1_HnaElement,HnaGlobalEntry*>::iterator it;
        it = hnaMap.find(*e);

        if (it==hnaMap.end())
            continue;

        hna_global_entry = it->second;

        /* if the given orig node is not in use no routes need to change */
        if (hna_global_entry->curr_orig_node == orig_node)
            continue;

        /* the TQ value has to better than the currently selected orig node */
        if (hna_global_entry->curr_orig_node->router->tq_avg > orig_node->router->tq_avg)
            continue;

        /**
         * if we change the orig node towards the HNA we may still route via the same next hop
         * which does not require any routing table changes
         */
        if (hna_global_entry->curr_orig_node->router->addr == orig_node->router->addr)
            goto set_orig_node;

        // should delete before add, see ManetRoutingBase::setRoute()
        add_del_route(e->addr, e->netmask, hna_global_entry->curr_orig_node->router->addr,
                hna_global_entry->curr_orig_node->router->if_incoming->if_index,
                hna_global_entry->curr_orig_node->router->if_incoming->dev,
                BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);

        add_del_route(e->addr, e->netmask, orig_node->router->addr,
                orig_node->router->if_incoming->if_index,
                orig_node->router->if_incoming->dev,
                BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);

set_orig_node:
        hna_global_entry->curr_orig_node = orig_node;
    }
}

void BatMobileV1::hna_global_del(BatMobileV1_OrigNode *orig_node)
{
    BatMobileV1_HnaElement e;

    if (orig_node->hna_buff.empty())
        return;

    /* delete routes */
    while (!orig_node->hna_buff.empty()) {
        e = orig_node->hna_buff.back();
        orig_node->hna_buff.pop_back();

        /* not found / deleted, need to add this new route */
        if ((e.netmask > 0) && (e.netmask <= 32))
            _hna_global_del(orig_node, &e);
    }
}

#if 0
static void _hna_global_hash_del(void *data)
{
    HnaGlobalEntry *hna_global_entry = data;
    struct hna_orig_ptr *hna_orig_ptr = NULL;
    struct list_head *list_pos, *list_pos_tmp;

    list_for_each_safe(list_pos, list_pos_tmp, &hna_global_entry->orig_list) {
        hna_orig_ptr = list_entry(list_pos, struct hna_orig_ptr, list);

        list_del((struct list_head *)&hna_global_entry->orig_list, list_pos, &hna_global_entry->orig_list);
        delete hna_orig_ptr;
    }

    delete hna_global_entry;
}
#endif

void BatMobileV1::hna_free(void)
{
    BatMobileV1_HnaLocalEntry *hna_local_entry;

    /* hna local */
    while (!hna_list.empty()) {
        hna_local_entry = &hna_list.back();
        hna_local_update_routes(hna_local_entry, ROUTE_DEL);

        hna_list.pop_back();
    }

    hna_buff_local.clear();


    /* hna global */
    while (!hnaMap.empty()) {
        delete hnaMap.begin()->second;
        hnaMap.erase(hnaMap.begin());
    }
}
