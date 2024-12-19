/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OHOS_VPN_ENCRYPTION_UTIL_H
#define OHOS_VPN_ENCRYPTION_UTIL_H
#include <string>
#include <vector>
#include "hks_api.h"
#include "hks_type.h"
#include "hks_param.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr const char *ENCRYT_KEY_FILENAME = "SysVpn";
constexpr const char *ENCRYT_SPLIT_SEP = ",";
constexpr uint32_t AES_COMMON_SIZE = 2048 + 16; // 2048 for AES-256, 16 for IV
constexpr uint32_t AAD_SIZE = 16;
constexpr uint32_t NONCE_SIZE = 16;
constexpr uint32_t AEAD_SIZE = 16;
constexpr uint32_t AES_256_NONCE_SIZE = 32;
constexpr uint32_t MAX_UPDATE_SIZE = 64 * 1024;

const uint8_t AAD[AAD_SIZE] = {0};

class EncryptedData final {
public:
    std::string encryptedData_ = "";
    std::string iv_ = "";
    EncryptedData(const std::string data, const std::string inputIv)
    {
        encryptedData_ = data;
        iv_ = inputIv;
    }
    EncryptedData() {}
    ~EncryptedData() {}
};

class VpnEncryptionInfo {
public:
    int32_t userId = -1;
    std::string fileName;
    static constexpr char SYSVPN_ENCRY_KEY[] = "EncryHksAes";
    struct HksBlob keyAlias;
    void SetFile(const std::string file, int32_t id)
    {
        fileName = SYSVPN_ENCRY_KEY + file;
        keyAlias = { fileName.length(), (uint8_t *)&fileName[0] };
        userId = id;
    }
    explicit VpnEncryptionInfo(const std::string file, int32_t id)
    {
        SetFile(file, id);
    }
    VpnEncryptionInfo() {}
    ~VpnEncryptionInfo() {}
};

/**
 * @Description  Set up Huks service
 * @return HKS_SUCCESS - setup success, others - setup failed
 */
int32_t SetUpHks();

/**
 * @Description  Generate new or get existed GCM-AES key based on input encryptionInfo and genParamSet
 * @param keyAlias  - keyAlias info
 * @param genParamSet - generate params
 * @return HKS_SUCCESS - find key, others - find key failed
 */
int32_t GetKeyByAlias(struct HksBlob *keyAlias, const struct HksParamSet *genParamSet);

/**
 * @Description  Encrypt inputString using GCM-AES based on input encryptionInfo
 * @param VpnEncryptionInfo  - keyAlias info
 * @param inputString - plaint string that needs to be encrypted
 * @param encryptedData - encrypted result with encrypted string and IV value
 * @return HKS_SUCCESS - encryption success, others - encryption failed
 */
int32_t VpnEncryption(const VpnEncryptionInfo &vpnEncryptionInfo, const std::string &inputString,
    EncryptedData &encryptedData);

/**
 * @Description  Decrypt encryptedData using GCM-AES based on input encryptionInfo
 * @param VpnEncryptionInfo  - keyAlias info
 * @param encryptedData - encrypted result with encrypted string and IV value
 * @param decryptedData - string after decryption
 * @return HKS_SUCCESS - decryption success, others - decryption failed
 */
int32_t VpnDecryption(const VpnEncryptionInfo &vpnEncryptionInfo, const EncryptedData &encryptedData,
    std::string &decryptedData);

/**
 * @Description  Encrypt string using GCM-AES based on input encryptionInfo
 * @param VpnEncryptionInfo  - keyAlias info
 * @param data - Encrypt string
 * @return HKS_SUCCESS - encryption success, others - encryption failed
 */
int32_t VpnEncryptData(const VpnEncryptionInfo &vpnEncryptionInfo, std::string &data);

/**
 * @Description  Decrypt string using GCM-AES based on input encryptionInfo
 * @param VpnEncryptionInfo  - keyAlias info
 * @param data - Decrypt string
 * @return HKS_SUCCESS - decryption success, others - decryption failed
 */
int32_t VpnDecryptData(const VpnEncryptionInfo &vpnEncryptionInfo, std::string &data);

} // namespace NetManagerStandard
} // namespace OHOS

#endif // OHOS_VPN_ENCRYPTION_UTIL_H