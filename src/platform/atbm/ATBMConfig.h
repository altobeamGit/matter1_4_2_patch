/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    All rights reserved.
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
 *    @file
 *          Utilities for interacting with the the ATBM Easyflash module.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides functions and definitions for accessing device configuration information on the ATBM platform.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class ATBMConfig
{
public:
    struct Key;

    // Maximum length of an easyflash key name
    static constexpr size_t kMaxConfigKeyNameLength = 15;

    // Easyflash namespaces used to store device configuration information.
    static const char kConfigNamespace_ChipFactory[];
    static const char kConfigNamespace_ChipConfig[];
    static const char kConfigNamespace_ChipCounters[];

    // Key definitions for well-known keys.
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_HardwareVersion;
    static const Key kConfigKey_HardwareVersionString;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_Spake2pIterationCount;
    static const Key kConfigKey_Spake2pSalt;
    static const Key kConfigKey_Spake2pVerifier;
    static const Key kConfigKey_DACCert;
    static const Key kConfigKey_DACPrivateKey;
    static const Key kConfigKey_DACPublicKey;
    static const Key kConfigKey_PAICert;
    static const Key kConfigKey_CertDeclaration;
    static const Key kConfigKey_VendorId;
    static const Key kConfigKey_VendorName;
    static const Key kConfigKey_ProductId;
    static const Key kConfigKey_ProductName;
    static const Key kConfigKey_ProductLabel;
    static const Key kConfigKey_PartNumber;
    static const Key kConfigKey_ProductURL;
    static const Key kConfigKey_SupportedCalTypes;
    static const Key kConfigKey_SupportedLocaleSize;
    static const Key kConfigKey_RotatingDevIdUniqueId;
    static const Key kConfigKey_ProductFinish;
    static const Key kConfigKey_ProductColor;
    static const Key kConfigKey_LocationCapability;
    static const Key kConfigKey_PrimaryDeviceType;

    // CHIP Config keys
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_UniqueId;
    static const Key kConfigKey_LockUser;
    static const Key kConfigKey_Credential;
    static const Key kConfigKey_LockUserName;
    static const Key kConfigKey_CredentialData;
    static const Key kConfigKey_UserCredentials;
    static const Key kConfigKey_WeekDaySchedules;
    static const Key kConfigKey_YearDaySchedules;
    static const Key kConfigKey_HolidaySchedules;

    // CHIP Counter keys
    static const Key kCounterKey_RebootCount;
    static const Key kCounterKey_UpTime;
    static const Key kCounterKey_TotalOperationalHours;

    // Config value accessors.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);

    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);

    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);


    static CHIP_ERROR EnsureNamespace(const char * ns);
    static CHIP_ERROR ClearNamespace(const char * ns);

    static void RunConfigUnitTest(void);
};

struct ATBMConfig::Key
{
    const char * name;

    bool operator==(const Key & other) const;
};

inline bool ATBMConfig::Key::operator==(const Key & other) const
{
    return strcmp(name, other.name) == 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
