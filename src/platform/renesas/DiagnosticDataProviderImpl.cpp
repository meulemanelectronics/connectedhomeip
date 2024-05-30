/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 * Provides an implementation of the DiagnosticDataProvider object for renesas platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include <app/data-model/List.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/renesas/DiagnosticDataProviderImpl.h>

#include "FreeRTOS.h"

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    size_t freeHeapSize = xPortGetFreeHeapSize();
    currentHeapFree     = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    // Calculate the Heap used based on Total heap - Free heap
    int64_t heapUsed = (configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize());

    // Something went wrong, this should not happen
    VerifyOrReturnError(heapUsed >= 0, CHIP_ERROR_INVALID_INTEGER_VALUE);
    currentHeapUsed = static_cast<uint64_t>(heapUsed);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    // FreeRTOS records the lowest amount of available heap during runtime
    // currentHeapHighWatermark wants the highest heap usage point so we calculate it here
    int64_t HighestHeapUsageRecorded = (configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize());

    // Something went wrong, this should not happen
    VerifyOrReturnError(HighestHeapUsageRecorded >= 0, CHIP_ERROR_INVALID_INTEGER_VALUE);
    currentHeapHighWatermark = static_cast<uint64_t>(HighestHeapUsageRecorded);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;
    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);
    if (err == CHIP_NO_ERROR)
    {
        // If the value overflows, return UINT16 max value to provide best-effort number.
        rebootCount = static_cast<uint16_t>(count <= UINT16_MAX ? count : UINT16_MAX);
    }
    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = 0;
    CHIP_ERROR err  = ConfigurationMgr().GetBootReason(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    NetworkInterface *head = nullptr;

    for (Inet::InterfaceIterator interfaceIterator; interfaceIterator.HasCurrent(); interfaceIterator.Next())
    {
        char interfaceName[Inet::InterfaceId::kMaxIfNameLength];
        interfaceIterator.GetInterfaceName(interfaceName, sizeof(interfaceName));
        struct netif *net_interface = netif_find(interfaceName);
        if (!net_interface)
        {
            continue;
        }

        NetworkInterface *ifp = new NetworkInterface();
        if (ifp == nullptr)
        {
            err = CHIP_ERROR_NO_MEMORY;
            break;
        }

        ifp->name = CharSpan::fromCharString(net_interface->name);
        ifp->isOperational = (net_interface->flags & NETIF_FLAG_LINK_UP) != 0;
        ifp->type = EMBER_ZCL_INTERFACE_TYPE_ENUM_ETHERNET;
        ifp->offPremiseServicesReachableIPv4.SetNull();
        ifp->offPremiseServicesReachableIPv6.SetNull();
        ifp->hardwareAddress = ByteSpan(net_interface->hwaddr, net_interface->hwaddr_len);

        size_t ipv6AddressCount = 0;

        Inet::InterfaceAddressIterator interfaceAddressIterator;
        while (interfaceAddressIterator.HasCurrent())
        {
            if (interfaceAddressIterator.GetInterfaceId() == interfaceIterator.GetInterfaceId())
            {
                Inet::IPAddress ipAddress;
                if (interfaceAddressIterator.GetAddress(ipAddress) == CHIP_NO_ERROR)
                {
                    if (ipAddress.IsIPv6() && ipv6AddressCount < kMaxIPv6AddrCount)
                    {
                        memcpy(ifp->Ipv6AddressesBuffer[ipv6AddressCount], ipAddress.Addr, sizeof(ipAddress.Addr));
                        ifp->Ipv6AddressSpans[ipv6AddressCount] = ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressCount], sizeof(ipAddress.Addr));
                        ipv6AddressCount++;
                    }
                }
            }
            interfaceAddressIterator.Next();
        }

        if (ipv6AddressCount > 0)
        {
            ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressCount);
        }

        ifp->Next = head;
        head = ifp;
    }

    if (head == nullptr)
    {
        err = CHIP_ERROR_NOT_FOUND;
    }

    *netifpp = head;

    return err;
}

void DiagnosticDataProviderImpl::ReleaseNetworkInterfaces(NetworkInterface * netifp)
{
    while (netifp)
    {
        NetworkInterface * del = netifp;
        netifp                 = netifp->Next;
        delete del;
    }
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
