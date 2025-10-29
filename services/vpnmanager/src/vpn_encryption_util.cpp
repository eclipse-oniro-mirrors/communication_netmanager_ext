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
#include "vpn_encryption_util.h"

#include <iterator>
#include <securec.h>
#include <sstream>

#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
struct HksParam g_genParam[] = {
    { .tag = HKS_TAG_SPECIFIC_USER_ID, .int32Param = 0 },
    { .tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT },
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE },
    { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
    { .tag = HKS_TAG_IS_KEY_ALIAS, .boolParam = true },
    { .tag = HKS_TAG_KEY_GENERATE_TYPE, .uint32Param = HKS_KEY_GENERATE_TYPE_DEFAULT },
    { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
    { .tag = HKS_TAG_AUTH_STORAGE_LEVEL, .uint32Param = HKS_AUTH_STORAGE_LEVEL_CE },
    { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = { .size = AAD_SIZE, .data = (uint8_t *)AAD } },
};

static char ConvertArrayChar(uint8_t ch)
{
    constexpr int maxDecNum = 9;
    constexpr int numDiffForHexAlphabet = 10;
    if (ch <= maxDecNum) {
        return '0' + ch;
    }
    if (ch <= 0xf) {
        return ch + 'a' - numDiffForHexAlphabet;
    }
    return '0';
}

static int8_t IsValidHexCharAndConvert(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + ('9' - '0' + 1);
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + ('9' - '0' + 1);
    }
    return -1;
}

std::string ConvertArrayToHex(const uint8_t plainText[], uint32_t size)
{
    constexpr int bitWidth = 4;
    std::stringstream ss;
    for (uint32_t i = 0; i < size; i++) {
        ss << ConvertArrayChar(plainText[i] >> bitWidth) << ConvertArrayChar (plainText[i] & 0xf);
    }
    return ss.str();
}

int HexStringToVec(const std::string &str, std::vector<char> &vec)
{
    unsigned len = str.length();
    if ((len & 1) != 0) {
        return -1;
    }
    const int hexShiftNum = 4;
    const int hexOffsetNum = 2;
    for (unsigned i = 0; i + 1 < len;) {
        uint8_t high = static_cast<uint8_t>(IsValidHexCharAndConvert(str[i]));
        uint8_t low = static_cast<uint8_t>(IsValidHexCharAndConvert(str[i + 1]));
        if (high < 0 || low < 0) {
            return -1;
        }
        char tmp = ((high << hexShiftNum) | (low & 0x0F));
        vec.push_back(tmp);
        i += hexOffsetNum;
    }
    return 0;
}

int HexStringToVec(const std::string &str, uint8_t plainText[], uint32_t plainLength, uint32_t &resultLength)
{
    std::vector<char> result;
    result.clear();
    int ret = HexStringToVec(str, result);
    if (ret == -1 || result.size() > plainLength) {
        return -1;
    }
    for (std::vector<char>::size_type i = 0; i < result.size(); ++i) {
        plainText[i] = result[i];
    }
    resultLength = result.size();
    return 0;
}

std::vector<std::string> Split(const std::string &str, const std::string &sep)
{
    std::string s = str;
    std::vector<std::string> res;
    while (!s.empty()) {
        size_t pos = s.find(sep);
        if (pos == std::string::npos) {
            res.emplace_back(s);
            break;
        }
        res.emplace_back(s.substr(0, pos));
        s = s.substr(pos + sep.size());
    }
    return res;
}

int32_t SetUpHks()
{
    int32_t ret = HksInitialize();
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn encryption init failed");
    }
    return ret;
}

int32_t GetKeyByAlias(struct HksBlob *keyAlias, const struct HksParamSet *genParamSet)
{
    if (keyAlias == nullptr || genParamSet == nullptr) {
        NETMGR_EXT_LOG_E("%{public}s invalid param", __func__);
        return -1;
    }
    int32_t keyExist = HksKeyExist(keyAlias, genParamSet);
    if (keyExist == HKS_ERROR_NOT_EXIST) {
        int32_t ret = HksGenerateKey(keyAlias, genParamSet, nullptr);
        if (ret != HKS_SUCCESS) {
            NETMGR_EXT_LOG_E("%{public}s generate key failed:%{public}d", __func__, keyExist);
        }
        return ret;
    } else if (keyExist != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("%{public}s search key failed:%{public}d", __func__, keyExist);
        return keyExist;
    }
    return keyExist;
}

int32_t VpnBuildHksParamSet(struct HksParamSet **paramSet, int32_t userId, uint8_t *nonce, uint32_t nonceSize)
{
    struct HksParam IVParam[] = {
        { .tag = HKS_TAG_NONCE, .blob = { .size = nonceSize, .data = nonce } },
    };
    g_genParam[0].int32Param = userId;
    int32_t ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("HksInitParamSet failed");
        return ret;
    }
    ret = HksAddParams(*paramSet, g_genParam, sizeof(g_genParam) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("HksAddParams g_genParam failed");
        HksFreeParamSet(paramSet);
        return ret;
    }
    ret = HksAddParams(*paramSet, IVParam, sizeof(IVParam) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("HksAddParams IVParam failed");
        HksFreeParamSet(paramSet);
        return ret;
    }
    ret = HksBuildParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("HksBuildParamSet failed");
        HksFreeParamSet(paramSet);
        return ret;
    }
    return ret;
}

int32_t VpnEncryptData(const VpnEncryptionInfo &vpnEncryptionInfo, std::string &data)
{
    if (!data.empty()) {
        EncryptedData encryptedData;
        if (VpnEncryption(vpnEncryptionInfo, data, encryptedData) != HKS_SUCCESS) {
            NETMGR_EXT_LOG_E("VpnEncryption failed");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }
        data = encryptedData.encryptedData_ + ENCRYT_SPLIT_SEP + encryptedData.iv_;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDecryptData(const VpnEncryptionInfo &vpnEncryptionInfo, std::string &data)
{
    if (!data.empty()) {
        const std::vector<std::string> encryedDataStrs = Split(data, ENCRYT_SPLIT_SEP);
        if (encryedDataStrs.size() > 1) {
            EncryptedData *encryptedData = new (std::nothrow) EncryptedData(encryedDataStrs[0], encryedDataStrs[1]);
            if (encryptedData == nullptr) {
                NETMGR_EXT_LOG_E("new EncryptedData failed");
                return NETMANAGER_EXT_ERR_INTERNAL;
            }
            std::string decryptedData = "";
            if (VpnDecryption(vpnEncryptionInfo, *encryptedData, decryptedData) != HKS_SUCCESS) {
                NETMGR_EXT_LOG_E("VpnDecryption failed");
                delete encryptedData;
                encryptedData = nullptr;
                return NETMANAGER_EXT_ERR_INTERNAL;
            }
            data = decryptedData;
            delete encryptedData;
            encryptedData = nullptr;
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnEncryption(const VpnEncryptionInfo &vpnEncryptionInfo, const std::string &inputString,
    EncryptedData &encryptedData)
{
    if (inputString.length() == 0) {
        return HKS_SUCCESS;
    }
    struct HksBlob authId = vpnEncryptionInfo.keyAlias;
    struct HksBlob plainText = { inputString.length(), (uint8_t *)&inputString[0] };

    uint8_t nonce[NONCE_SIZE] = {0};
    struct HksBlob randomIV = {NONCE_SIZE, nonce};
    int32_t ret = HksGenerateRandom(NULL, &randomIV);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn encryption generate IV failed");
        return ret;
    }

    struct HksParamSet *encryParamSet = nullptr;
    ret = VpnBuildHksParamSet(&encryParamSet, vpnEncryptionInfo.userId, nonce, NONCE_SIZE);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("VpnBuildHksParamSet failed");
        return ret;
    }

    ret = GetKeyByAlias(&authId, encryParamSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn encryption failed");
        HksFreeParamSet(&encryParamSet);
        return ret;
    }

    uint8_t cipherBuf[AES_COMMON_SIZE] = {0};
    HksBlob cipherData = {
        .size = AES_COMMON_SIZE,
        .data = cipherBuf
    };

    ret = HksEncrypt(&authId, encryParamSet, &plainText, &cipherData);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("Hks encryption failed");
        HksFreeParamSet(&encryParamSet);
        return ret;
    }

    encryptedData.encryptedData_ = ConvertArrayToHex(cipherBuf, cipherData.size);
    encryptedData.iv_ = ConvertArrayToHex(nonce, NONCE_SIZE);
    HksFreeParamSet(&encryParamSet);
    return ret;
}

int32_t VpnDecryptionBack(const VpnEncryptionInfo &vpnEncryptionInfo, const EncryptedData &encryptedData,
    std::string &decryptedData)
{
    if (encryptedData.encryptedData_.size() == 0) {
        return HKS_SUCCESS;
    }
    struct HksBlob authId = vpnEncryptionInfo.keyAlias;
    uint8_t cipherBuf[AES_COMMON_SIZE] = {0};
    uint32_t length = AES_COMMON_SIZE;
    int32_t retStrToArrat = HexStringToVec(encryptedData.encryptedData_, cipherBuf, AES_COMMON_SIZE, length);
    if (retStrToArrat != 0) {
        return HKS_FAILURE;
    }

    uint8_t nonce[NONCE_SIZE] = {0};
    uint32_t lengthIV = NONCE_SIZE;
    retStrToArrat = HexStringToVec(encryptedData.iv_, nonce, NONCE_SIZE, lengthIV);
    if (retStrToArrat != 0) {
        return HKS_FAILURE;
    }

    struct HksBlob cipherData = { length, cipherBuf };
    struct HksParamSet *decryParamSet = nullptr;
    uint8_t nonceBack[NONCE_SIZE] = {0};
    int32_t ret = VpnBuildHksParamSet(&decryParamSet, vpnEncryptionInfo.userId, nonceBack, NONCE_SIZE);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("BuildHksParamSet failed");
        return ret;
    }

    ret = HksKeyExist(&authId, decryParamSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn decryption key not exist");
        HksFreeParamSet(&decryParamSet);
        return ret;
    }
    uint8_t plainBuff[AES_COMMON_SIZE] = {0};
    HksBlob plainText = {
        .size = AES_COMMON_SIZE,
        .data = plainBuff
    };

    ret = HksDecrypt(&authId, decryParamSet, &cipherData, &plainText);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("Hks decryption failed");
        HksFreeParamSet(&decryParamSet);
        return ret;
    }

    std::string temp(plainText.data, plainText.data + plainText.size);
    decryptedData = temp;
    HksFreeParamSet(&decryParamSet);
    return ret;
}

int32_t VpnDecryption(const VpnEncryptionInfo &vpnEncryptionInfo, const EncryptedData &encryptedData,
    std::string &decryptedData)
{
    if (encryptedData.encryptedData_.size() == 0) {
        return HKS_SUCCESS;
    }
    struct HksBlob authId = vpnEncryptionInfo.keyAlias;
    uint8_t cipherBuf[AES_COMMON_SIZE] = {0};
    uint32_t length = AES_COMMON_SIZE;
    int32_t retStrToArrat = HexStringToVec(encryptedData.encryptedData_, cipherBuf, AES_COMMON_SIZE, length);
    if (retStrToArrat != 0) {
        return HKS_FAILURE;
    }

    uint8_t nonce[NONCE_SIZE] = {0};
    uint32_t lengthIV = NONCE_SIZE;
    retStrToArrat = HexStringToVec(encryptedData.iv_, nonce, NONCE_SIZE, lengthIV);
    if (retStrToArrat != 0) {
        return HKS_FAILURE;
    }

    struct HksBlob cipherData = { length, cipherBuf };
    struct HksParamSet *decryParamSet = nullptr;
    int32_t ret = VpnBuildHksParamSet(&decryParamSet, vpnEncryptionInfo.userId, nonce, NONCE_SIZE);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("BuildHksParamSet failed");
        return ret;
    }

    ret = HksKeyExist(&authId, decryParamSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("vpn decryption key not exist");
        HksFreeParamSet(&decryParamSet);
        return ret;
    }
    uint8_t plainBuff[AES_COMMON_SIZE] = {0};
    HksBlob plainText = {
        .size = AES_COMMON_SIZE,
        .data = plainBuff
    };

    ret = HksDecrypt(&authId, decryParamSet, &cipherData, &plainText);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("Hks decryption failed");
        HksFreeParamSet(&decryParamSet);
        ret = VpnDecryptionBack(vpnEncryptionInfo, encryptedData, decryptedData);
        return ret;
    }

    std::string temp(plainText.data, plainText.data + plainText.size);
    decryptedData = temp;
    HksFreeParamSet(&decryParamSet);
    return ret;
}

}  // namespace NetManagerStandard
}  // namespace OHOS