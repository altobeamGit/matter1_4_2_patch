/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <platform/atbm/ATBMConfig.h>
#include <platform/atbm/ATBMSecureCertDACProvider.h>

#if CONFIG_USE_ATBM_ECDSA_PERIPHERAL
#include <platform/atbm/ATBMCHIPCryptoPAL.h>
#endif // CONFIG_USE_ATBM_ECDSA_PERIPHERAL

#if CONFIG_SEC_CERT_DAC_PROVIDER

namespace chip {
namespace DeviceLayer {

using namespace chip::Credentials;
using namespace chip::DeviceLayer::Internal;

namespace {
static constexpr uint32_t kDACPrivateKeySize = 32;
static constexpr uint32_t kDACPublicKeySize  = 65;
static constexpr uint8_t kPrivKeyOffset      = 7;
static constexpr uint8_t kPubKeyOffset       = 56;

CHIP_ERROR LoadKeypairFromRaw(ByteSpan privateKey, ByteSpan publicKey, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serializedKeypair;
    ReturnErrorOnFailure(serializedKeypair.SetLength(privateKey.size() + publicKey.size()));
    memcpy(serializedKeypair.Bytes(), publicKey.data(), publicKey.size());
    memcpy(serializedKeypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return keypair.Deserialize(serializedKeypair);
}
} // namespace

CHIP_ERROR ATBMSecureCertDACProvider ::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    size_t certSize;
    ReturnErrorOnFailure(
        ATBMConfig::ReadConfigValueBin(ATBMConfig::kConfigKey_CertDeclaration, outBuffer.data(), outBuffer.size(), certSize));
    outBuffer.reduce_size(certSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMSecureCertDACProvider ::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // We do not provide any FirmwareInformation.
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ATBMSecureCertDACProvider ::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    char * dac_cert  = NULL;
    uint32_t dac_len = 0;

    int err = atbm_secure_cert_get_device_cert(&dac_cert, &dac_len);
    if (err == 0 && dac_cert != NULL && dac_len != 0)
    {
        VerifyOrReturnError(dac_len <= kMaxDERCertLength, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT,
                            atbm_secure_cert_free_device_cert(dac_cert));
        VerifyOrReturnError(dac_len <= outBuffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL, atbm_secure_cert_free_device_cert(dac_cert));
        memcpy(outBuffer.data(), dac_cert, outBuffer.size());
        outBuffer.reduce_size(dac_len);
        atbm_secure_cert_free_device_cert(dac_cert);
        return CHIP_NO_ERROR;
    }

    ChipLogError(DeviceLayer, "atbm_secure_cert_get_device_cert failed err:%d", err);
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR ATBMSecureCertDACProvider ::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    char * pai_cert  = NULL;
    uint32_t pai_len = 0;
    int err    = atbm_secure_cert_get_ca_cert(&pai_cert, &pai_len);
    if (err == 0 && pai_cert != NULL && pai_len != 0)
    {
        VerifyOrReturnError(pai_len <= kMaxDERCertLength, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT,
                            atbm_secure_cert_free_ca_cert(pai_cert));
        VerifyOrReturnError(pai_len <= outBuffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL, atbm_secure_cert_free_ca_cert(pai_cert));
        memcpy(outBuffer.data(), pai_cert, outBuffer.size());
        outBuffer.reduce_size(pai_len);
        atbm_secure_cert_free_ca_cert(pai_cert);
        return CHIP_NO_ERROR;
    }

    ChipLogError(DeviceLayer, "atbm_secure_cert_get_ca_cert failed err:%d", err);
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR ATBMSecureCertDACProvider ::SignWithDeviceAttestationKey(const ByteSpan & messageToSign,
                                                                     MutableByteSpan & outSignBuffer)
{
    int err;
    atbm_secure_cert_key_type_t keyType;

    CHIP_ERROR chipError;
    Crypto::P256ECDSASignature signature;

    VerifyOrReturnError(!outSignBuffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!messageToSign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    err = atbm_secure_cert_get_priv_key_type(&keyType);
    VerifyOrReturnError(err == 0, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "Failed to get the type of private key from secure cert partition, err:%d", err));

    VerifyOrReturnError(keyType != ATBM_SECURE_CERT_INVALID_KEY, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "Private key type in secure cert partition is invalid"));

    // This flow is for devices supporting ECDSA peripheral
    if (keyType == ATBM_SECURE_CERT_ECDSA_PERIPHERAL_KEY)
    {
#if CONFIG_USE_ATBM_ECDSA_PERIPHERAL
        Crypto::ATBMP256Keypair keypair;
        uint8_t efuseBlockId;

        err = atbm_secure_cert_get_priv_key_efuse_id(&efuseBlockId);
        VerifyOrReturnError(err == 0, CHIP_ERROR_INVALID_KEY_ID,
                            ChipLogError(DeviceLayer, "Failed to get the private key efuse block id, err:%d", err));

        ChipLogProgress(DeviceLayer, "efuse block id:%u", efuseBlockId);

        chipError = keypair.Initialize(chip::Crypto::ECPKeyTarget::ECDSA, efuseBlockId);
        VerifyOrReturnError(chipError == CHIP_NO_ERROR, chipError,
                            ChipLogError(DeviceLayer, "Failed to initialize the keypair err:%" CHIP_ERROR_FORMAT, chipError.Format()));

        chipError = keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature);
        VerifyOrReturnError(
            chipError == CHIP_NO_ERROR, chipError,
            ChipLogError(DeviceLayer, "Failed to sign with device attestation key, err:%" CHIP_ERROR_FORMAT, chipError.Format()));
#else
        return CHIP_ERROR_INCORRECT_STATE;
#endif // CONFIG_USE_ATBM_ECDSA_PERIPHERAL
    }
    else // This flow is for devices which do not support ECDSA peripheral
    {
#if !CONFIG_USE_ATBM_ECDSA_PERIPHERAL
        Crypto::P256Keypair keypair;
        char * sc_keypair       = NULL;
        uint32_t sc_keypair_len = 0;

        err = atbm_secure_cert_get_priv_key(&sc_keypair, &sc_keypair_len);
        VerifyOrReturnError(err == 0 && sc_keypair != NULL && sc_keypair_len != 0, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(DeviceLayer, "atbm_secure_cert_get_priv_key failed err:%d", err));

        chipError =
            LoadKeypairFromRaw(ByteSpan(reinterpret_cast<const uint8_t *>(sc_keypair + kPrivKeyOffset), kDACPrivateKeySize),
                               ByteSpan(reinterpret_cast<const uint8_t *>(sc_keypair + kPubKeyOffset), kDACPublicKeySize), keypair);
        VerifyOrReturnError(chipError == CHIP_NO_ERROR, chipError, atbm_secure_cert_free_priv_key(sc_keypair));

        chipError = keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature);
        VerifyOrReturnError(chipError == CHIP_NO_ERROR, chipError, atbm_secure_cert_free_priv_key(sc_keypair));

        atbm_secure_cert_free_priv_key(sc_keypair);
#else
        return CHIP_ERROR_INCORRECT_STATE;
#endif // !CONFIG_USE_ESP32_ECDSA_PERIPHERAL
    }
    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

} // namespace DeviceLayer
} // namespace chip

#endif // CONFIG_SEC_CERT_DAC_PROVIDER
