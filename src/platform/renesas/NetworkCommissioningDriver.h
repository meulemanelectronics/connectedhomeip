/**
 * @copyright Copyright 2024, (C) Sensorfy B.V.
 */

#pragma once

#include <lib/support/BitFlags.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

class RenesasEthernetDriver : public EthernetDriver
{
public:
    class EthernetNetworkIterator final : public NetworkIterator
    {
    public:
        EthernetNetworkIterator(RenesasEthernetDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() { return 1; }
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~EthernetNetworkIterator() = default;

    private:
        RenesasEthernetDriver * mDriver;
        bool exhausted = false;
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new EthernetNetworkIterator(this); }
    uint8_t GetMaxNetworks() override { return 1; }
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown() override;

    static RenesasEthernetDriver & GetInstance()
    {
        static RenesasEthernetDriver instance;
        return instance;
    }
    virtual ~RenesasEthernetDriver() = default;
};


} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
