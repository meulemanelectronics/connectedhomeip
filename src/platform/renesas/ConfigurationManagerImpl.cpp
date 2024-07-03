/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 * Provides the implementation of the Device Layer ConfigurationManager object for Renesas platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include "lwip/netif.h"

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
    CHIP_ERROR err;
    uint32_t rebootCount;
    err = GetRebootCount(rebootCount);
    SuccessOrExit(err);

    err = StoreRebootCount(rebootCount + 1);
    SuccessOrExit(err);

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<RenesasConfig>::Init();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    size_t readBytesSize;

    auto err = PersistedStorage::KeyValueStoreMgr().Get(
        DefaultStorageKeyAllocator::AttributeValue(0, chip::app::Clusters::GeneralDiagnostics::Id,
        chip::app::Clusters::GeneralDiagnostics::Attributes::RebootCount::Id).KeyName(), &rebootCount,
        sizeof(rebootCount), &readBytesSize);

    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        rebootCount = 0;
        return CHIP_NO_ERROR;
    }
    else
    {
        return err;
    }
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    auto err = PersistedStorage::KeyValueStoreMgr().Put(
        DefaultStorageKeyAllocator::AttributeValue(0, chip::app::Clusters::GeneralDiagnostics::Id,
        chip::app::Clusters::GeneralDiagnostics::Attributes::RebootCount::Id).KeyName(), &rebootCount,
        sizeof(rebootCount));

    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)
{
    size_t readBytesSize;
    auto err = PersistedStorage::KeyValueStoreMgr().Get(DefaultStorageKeyAllocator::AttributeValue(
        0, chip::app::Clusters::BasicInformation::Id,
        chip::app::Clusters::BasicInformation::Attributes::Location::Id).KeyName(), buf, bufSize, &readBytesSize);

    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        codeLen = 0;
        return CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        codeLen = readBytesSize;
    }

    return err;
}

CHIP_ERROR ConfigurationManagerImpl::StoreCountryCode(const char * code, size_t codeLen)
{
    auto err = PersistedStorage::KeyValueStoreMgr().Put(DefaultStorageKeyAllocator::AttributeValue(
        0, chip::app::Clusters::BasicInformation::Id,
        chip::app::Clusters::BasicInformation::Attributes::Location::Id).KeyName(), code, codeLen);
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    return ReadConfigValue(RenesasConfig::kConfigKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason)
{
    return WriteConfigValue(RenesasConfig::kConfigKey_BootReason, bootReason);
}

void ConfigurationManagerImpl::RegisterNetif(struct netif* netif)
{
        m_netif = netif;
}

bool ConfigurationManagerImpl::CanFactoryReset(void)
{
    // Implement InitiateFactoryReset first before enabling
    return false;
}

void ConfigurationManagerImpl::InitiateFactoryReset(void)
{
    // TODO implement
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH + 1);
    VerifyOrReturnError(keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    return PersistedStorage::KeyValueStoreMgr().Get(key, &value, sizeof(value));
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH + 1);
    VerifyOrReturnError(keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    return PersistedStorage::KeyValueStoreMgr().Put(key, &value, sizeof(value));
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return RenesasConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return RenesasConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return RenesasConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return RenesasConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return RenesasConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return RenesasConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return RenesasConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return RenesasConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return RenesasConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return RenesasConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return RenesasConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    RenesasConfig::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    // Implement factory reset
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    if (m_netif != nullptr && buf != nullptr)
    {
        memcpy(buf, m_netif->hwaddr, m_netif->hwaddr_len);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace chip
