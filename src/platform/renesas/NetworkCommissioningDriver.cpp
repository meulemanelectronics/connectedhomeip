/**
 * @copyright Copyright 2024, (C) Sensorfy B.V.
 */

#include <platform/renesas/NetworkCommissioningDriver.h>
#include <lwip/netif.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {


netif* FindEthernetItf()
{
    for(auto itf = netif_list; itf; itf = itf->next) {
        if (itf->flags & (NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP)) {
            return itf;
        }
    }
    return nullptr;
}

bool RenesasEthernetDriver::EthernetNetworkIterator::Next(Network & item)
{
    if (exhausted)
        return false;

    auto netif = FindEthernetItf();
    if (!netif) 
        return false;

    char buf[NETIF_NAMESIZE];
    char * ifname = netif_index_to_name(netif_get_index(netif), buf);

    item.networkIDLen = static_cast<uint8_t>(strlen(ifname));
    memcpy(item.networkID, ifname, item.networkIDLen);
    item.connected = netif_is_up(netif);

    exhausted = true;
    return true;
}

CHIP_ERROR RenesasEthernetDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    // TODO: this could call ethernet_interface_bringup() (netif_set_up) instead of thread_manager 
    // the callback should be remembered to signal Matter of changes
    return CHIP_NO_ERROR;
}   
void RenesasEthernetDriver::Shutdown()
{
    // TODO: This could call ethernet_interface_bringdown (netif_remove) if Ethernet is used alongside Wifi/Thread.
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
