/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>
#include <platform/atbm/ATBMConfig.h>
#include <platform/atbm/ATBMFactoryDataProvider.h>

namespace chip {
namespace DeviceLayer {

using namespace chip::Credentials;
using namespace chip::DeviceLayer::Internal;

namespace {
static constexpr uint32_t kDACPrivateKeySize = 32;
static constexpr uint32_t kDACPublicKeySize  = 65;

CHIP_ERROR LoadKeypairFromRaw(ByteSpan privateKey, ByteSpan publicKey, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serializedKeypair;
    ReturnErrorOnFailure(serializedKeypair.SetLength(privateKey.size() + publicKey.size()));
    memcpy(serializedKeypair.Bytes(), publicKey.data(), publicKey.size());
    memcpy(serializedKeypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return keypair.Deserialize(serializedKeypair);
}
} // namespace

CHIP_ERROR ATBMFactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t setupDiscriminator32;
    ReturnErrorOnFailure(ATBMConfig::ReadConfigValue(ATBMConfig::kConfigKey_SetupDiscriminator, setupDiscriminator32));
    VerifyOrReturnLogError(setupDiscriminator32 <= kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);
    setupDiscriminator = static_cast<uint16_t>(setupDiscriminator32);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMFactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    return ATBMConfig::ReadConfigValue(ATBMConfig::kConfigKey_Spake2pIterationCount, iterationCount);
}

CHIP_ERROR ATBMFactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_NO_ERROR;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

    err = ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_Spake2pSalt, saltB64, sizeof(saltB64), saltB64Len);
    ReturnErrorOnFailure(err);

    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));
    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMFactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    CHIP_ERROR err                                            = CHIP_NO_ERROR;
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    size_t verifierB64Len                                     = 0;

    err =
        ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_Spake2pVerifier, verifierB64, sizeof(verifierB64), verifierB64Len);
    ReturnErrorOnFailure(err);

    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMFactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    size_t certSize;
    ReturnErrorOnFailure(
        ATBMConfig::ReadConfigValueBin(ATBMConfig::kConfigKey_CertDeclaration, outBuffer.data(), outBuffer.size(), certSize));
    outBuffer.reduce_size(certSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMFactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // We do not provide any FirmwareInformation.
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMFactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    size_t certSize;
    ReturnErrorOnFailure(
        ATBMConfig::ReadConfigValueBin(ATBMConfig::kConfigKey_DACCert, outBuffer.data(), outBuffer.size(), certSize));
    outBuffer.reduce_size(certSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMFactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    size_t certSize;
    ReturnErrorOnFailure(
        ATBMConfig::ReadConfigValueBin(ATBMConfig::kConfigKey_PAICert, outBuffer.data(), outBuffer.size(), certSize));
    outBuffer.reduce_size(certSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMFactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(!outSignBuffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!messageToSign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t privKeyBuf[kDACPrivateKeySize];
    uint8_t pubKeyBuf[kDACPublicKeySize];
    size_t privKeyLen = sizeof(privKeyBuf);
    size_t pubKeyLen  = sizeof(pubKeyBuf);

    ReturnErrorOnFailure(
        ATBMConfig::ReadConfigValueBin(ATBMConfig::kConfigKey_DACPrivateKey, privKeyBuf, privKeyLen, privKeyLen));
    ReturnErrorOnFailure(ATBMConfig::ReadConfigValueBin(ATBMConfig::kConfigKey_DACPublicKey, pubKeyBuf, pubKeyLen, pubKeyLen));

    ReturnErrorOnFailure(LoadKeypairFromRaw(ByteSpan(privKeyBuf, privKeyLen), ByteSpan(pubKeyBuf, pubKeyLen), keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
CHIP_ERROR ATBMFactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    size_t vendorNameLen = 0; // without counting null-terminator
    return ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_VendorName, buf, bufSize, vendorNameLen);
}

CHIP_ERROR ATBMFactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    ChipError err   = CHIP_NO_ERROR;
    uint32_t valInt = 0;

    err = ATBMConfig::ReadConfigValue(ATBMConfig::kConfigKey_VendorId, valInt);
    ReturnErrorOnFailure(err);
    vendorId = static_cast<uint16_t>(valInt);
    return err;
}

CHIP_ERROR ATBMFactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    size_t productNameLen = 0; // without counting null-terminator
    return ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_ProductName, buf, bufSize, productNameLen);
}

CHIP_ERROR ATBMFactoryDataProvider::GetProductId(uint16_t & productId)
{
    ChipError err   = CHIP_NO_ERROR;
    uint32_t valInt = 0;

    err = ATBMConfig::ReadConfigValue(ATBMConfig::kConfigKey_ProductId, valInt);
    ReturnErrorOnFailure(err);
    productId = static_cast<uint16_t>(valInt);
    return err;
}

CHIP_ERROR ATBMFactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    CHIP_ERROR err = ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_ProductURL, buf, bufSize, bufSize);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ATBMFactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    CHIP_ERROR err = ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_ProductLabel, buf, bufSize, bufSize);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ATBMFactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    size_t hardwareVersionStringLen = 0; // without counting null-terminator
    return ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_HardwareVersionString, buf, bufSize, hardwareVersionStringLen);
}

CHIP_ERROR ATBMFactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ChipError err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");

    size_t uniqueIdLen = 0;
    err = ATBMConfig::ReadConfigValueBin(ATBMConfig::kConfigKey_RotatingDevIdUniqueId, uniqueIdSpan.data(), uniqueIdSpan.size(),
                                          uniqueIdLen);
    ReturnErrorOnFailure(err);
    uniqueIdSpan.reduce_size(uniqueIdLen);
#endif
    return err;
}

CHIP_ERROR ATBMFactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
	return GenericDeviceInstanceInfoProvider<ATBMConfig>::GetSerialNumber(buf, bufSize);
}

CHIP_ERROR ATBMFactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
	return GenericDeviceInstanceInfoProvider<ATBMConfig>::GetManufacturingDate(year, month, day);
}

CHIP_ERROR ATBMFactoryDataProvider::GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint32_t productFinish = 0;

    err = ATBMConfig::ReadConfigValue(ATBMConfig::kConfigKey_ProductFinish, productFinish);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *finish = static_cast<app::Clusters::BasicInformation::ProductFinishEnum>(productFinish);

    return err;
}

CHIP_ERROR ATBMFactoryDataProvider::GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t color = 0;

    err = ATBMConfig::ReadConfigValue(ATBMConfig::kConfigKey_ProductColor, color);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NOT_IMPLEMENTED);

    *primaryColor = static_cast<app::Clusters::BasicInformation::ColorEnum>(color);

    return err;
}

CHIP_ERROR ATBMFactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
	return GenericDeviceInstanceInfoProvider<ATBMConfig>::GetHardwareVersion(hardwareVersion);
}

CHIP_ERROR ATBMFactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    CHIP_ERROR err = ATBMConfig::ReadConfigValueStr(ATBMConfig::kConfigKey_PartNumber, buf, bufSize, bufSize);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER

} // namespace DeviceLayer
} // namespace chip
