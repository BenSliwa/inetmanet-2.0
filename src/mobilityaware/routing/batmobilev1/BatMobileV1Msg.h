#ifndef __INET_BATMANMSG_H_
#define __INET_BATMANMSG_H_

#include "INETDefs.h"

#include "BatMobileV1Msg_m.h"


inline bool operator < (BatMobileV1_HnaElement const &a, BatMobileV1_HnaElement const &b)
{
    return (a.addr < b.addr) || (a.addr == b.addr && a.netmask < b.netmask);
}

inline bool operator > (BatMobileV1_HnaElement const &a, BatMobileV1_HnaElement const &b)
{
    return (a.addr > b.addr) || (a.addr == b.addr && a.netmask > b.netmask);
}

inline bool operator == (BatMobileV1_HnaElement const &a, BatMobileV1_HnaElement const &b)
{
    return (a.addr == b.addr && a.netmask == b.netmask);
}

inline bool operator != (BatMobileV1_HnaElement const &a, BatMobileV1_HnaElement const &b)
{
    return !(a.addr == b.addr && a.netmask == b.netmask);
}


#endif // __INET_BATMANMSG_H_
