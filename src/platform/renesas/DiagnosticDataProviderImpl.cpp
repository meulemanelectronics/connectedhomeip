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
        if(NetworkInterface *ifp = new NetworkInterface())
        {
            interfaceIterator.GetInterfaceName(ifp->Name, Inet::InterfaceId::kMaxIfNameLength);

            ifp->name = CharSpan::fromCharString(ifp->Name);
            ifp->isOperational = interfaceIterator.IsUp();

            Inet::InterfaceType interfaceType;
            if (interfaceIterator.GetInterfaceType(interfaceType) == CHIP_NO_ERROR)
            {
                switch (interfaceType)
                {
                    case Inet::InterfaceType::Unknown:
                        ifp->type = EMBER_ZCL_INTERFACE_TYPE_ENUM_UNSPECIFIED;
                        break;
                    case Inet::InterfaceType::WiFi:
                        ifp->type = EMBER_ZCL_INTERFACE_TYPE_ENUM_WI_FI;
                        break;
                    case Inet::InterfaceType::Ethernet:
                        ifp->type = EMBER_ZCL_INTERFACE_TYPE_ENUM_ETHERNET;
                        break;
                    case Inet::InterfaceType::Thread:
                        ifp->type = EMBER_ZCL_INTERFACE_TYPE_ENUM_THREAD;
                        break;
                    case Inet::InterfaceType::Cellular:
                        ifp->type = EMBER_ZCL_INTERFACE_TYPE_ENUM_CELLULAR;
                        break;
                }
            }
            else
            {
                ChipLogError(DeviceLayer, "Failed to get interface type");
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_ENUM_UNSPECIFIED;
            }
            ifp->offPremiseServicesReachableIPv4.SetNull();
            ifp->offPremiseServicesReachableIPv6.SetNull();

            uint8_t addressSize = 0;
            err = interfaceIterator.GetHardwareAddress(ifp->MacAddress, addressSize, sizeof(ifp->MacAddress));

            if (err == CHIP_NO_ERROR)
            {
                ifp->hardwareAddress = ByteSpan(ifp->MacAddress, addressSize);
            }
            else
            {
                ChipLogError(DeviceLayer, "Failed to get network hardware address");
                ifp->hardwareAddress = ByteSpan();
            }

            size_t ipv6AddressCount = 0;

            Inet::InterfaceAddressIterator interfaceAddressIterator;
            for(Inet::InterfaceAddressIterator interfaceAddressIterator; interfaceAddressIterator.HasCurrent(); interfaceAddressIterator.Next())
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
            }

            if (ipv6AddressCount > 0)
            {
                ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressCount);
            }

            ifp->Next = head;
            head = ifp;
        }
        else
        {
            err = CHIP_ERROR_NO_MEMORY;
            break;
        }

    }

    if (err!=CHIP_NO_ERROR)
    {
       ReleaseNetworkInterfaces(head);
       return err;
    }
    else if (head == nullptr)
    {
       return CHIP_ERROR_NOT_FOUND;
    }
    else
    {
       *netifpp = head;
       return CHIP_NO_ERROR;
    }
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
