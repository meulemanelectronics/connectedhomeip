/**
 * @copyright Copyright 2024, (C) Sensorfy B.V.
 */

#include <lib/core/CHIPError.h>
#include <cstdint>

#pragma once

namespace chip {
namespace DeviceLayer {

class ConfigurationInformationProvider
{
public:
    virtual ~ConfigurationInformationProvider() = default;

    virtual CHIP_ERROR GetMacAddress(uint8_t* mac) = 0;
    virtual CHIP_ERROR GetSoftwareVersionString(char * buf, size_t bufSize) = 0;
    virtual CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVer) = 0;

};

}
}
