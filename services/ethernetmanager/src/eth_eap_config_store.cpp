/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "eth_eap_config_store.h"

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <securec.h>

#include "cJSON.h"
#include "hks_api.h"
#include "hks_type.h"
#include "hks_param.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

/** 802.1X 配置落盘路径（系统目录，普通应用不可访问） */
static constexpr const char *ETH_EAP_CONFIG_PATH = "/data/service/el1/public/eth/eth_eap_config.json";
/** 802.1X 配置所在目录 */
static constexpr const char *ETH_EAP_CONFIG_DIR = "/data/service/el1/public/eth";
/** HUKS 密钥别名前缀（拼接后得到完整别名） */
static constexpr const char *ETH_EAP_ENCRY_KEY = "EthEapEncryHksAes";
/** HUKS 密钥别名后缀（拼接后得到完整别名） */
static constexpr const char *ETH_EAP_ENCRY_FILE = "EthEapConfig";
/** 加密串中"密文"与"nonce"之间的分隔符 */
static constexpr const char *ENCRYT_SPLIT_SEP = ",";
/** GCM 认证标签（tag）长度 */
static constexpr uint32_t GCM_TAG_SIZE = 16;
/** GCM 认证附加数据（AAD）长度 */
static constexpr uint32_t AAD_SIZE = 16;
/** GCM 随机数（nonce）长度 */
static constexpr uint32_t NONCE_SIZE = 16;

/** AAD 固定全 0 数组（不随每次加密变化） */
const uint8_t AAD[AAD_SIZE] = {0};

/** HUKS 通用参数集：AES-256-GCM、持久化、DE 存储级别、按别名引用，首次使用自动生成 */
struct HksParam g_ethEapGenParam[] = {
    { .tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT },
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_NONE },
    { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
    { .tag = HKS_TAG_IS_KEY_ALIAS, .boolParam = true },
    { .tag = HKS_TAG_KEY_GENERATE_TYPE, .uint32Param = HKS_KEY_GENERATE_TYPE_DEFAULT },
    { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
    { .tag = HKS_TAG_AUTH_STORAGE_LEVEL, .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE },
    { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = { .size = AAD_SIZE, .data = (uint8_t *)AAD } },
};

/** 单字节转一位 hex 字符 */
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

/** 二进制数组转小写 hex 字符串 */
static std::string ConvertArrayToHex(const uint8_t plainText[], uint32_t size)
{
    constexpr int bitWidth = 4;
    std::stringstream ss;
    for (uint32_t i = 0; i < size; i++) {
        ss << ConvertArrayChar(plainText[i] >> bitWidth) << ConvertArrayChar(plainText[i] & 0xf);
    }
    return ss.str();
}

/** 合法 hex 字符转数值（0~15），非法返回 -1 */
static int8_t IsValidHexCharAndConvert(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

/** hex 字符串还原二进制数组（ConvertArrayToHex 逆操作），成功 0 失败 -1 */
static int HexStringToVec(const std::string &str, uint8_t plainText[], uint32_t plainLength, uint32_t &resultLength)
{
    unsigned len = str.length();
    if ((len & 1) != 0) {
        return -1;
    }
    const int hexShiftNum = 4;
    const int hexOffsetNum = 2;
    std::vector<uint8_t> result;
    for (unsigned i = 0; i + 1 < len;) {
        // 必须用有符号 int 接收，-1 才能被正确识别
        int high = IsValidHexCharAndConvert(str[i]);
        int low = IsValidHexCharAndConvert(str[i + 1]);
        if (high < 0 || low < 0) {
            return -1;
        }
        result.push_back((static_cast<uint8_t>(high) << hexShiftNum) | (static_cast<uint8_t>(low) & 0x0F));
        i += hexOffsetNum;
    }
    if (result.size() > plainLength) {
        return -1;
    }
    for (size_t i = 0; i < result.size(); ++i) {
        plainText[i] = result[i];
    }
    resultLength = result.size();
    return 0;
}

/**
 * 从 JSON 对象中读取字符串字段（字段缺失或类型不符时保持原值，容错解析）。
 */
static void ReadJsonString(cJSON *root, const char *name, std::string &out)
{
    cJSON *item = cJSON_GetObjectItem(root, name);
    if (item != nullptr && cJSON_IsString(item)) {
        out = item->valuestring;
    }
}

/**
 * 从 JSON 对象中读取整型字段（字段缺失或类型不符时保持原值，容错解析）。
 */
static void ReadJsonNumber(cJSON *root, const char *name, int32_t &out)
{
    cJSON *item = cJSON_GetObjectItem(root, name);
    if (item != nullptr && cJSON_IsNumber(item)) {
        out = item->valueint;
    }
}

/**
 * 从 JSON 对象中读取布尔字段（字段缺失或类型不符时保持原值，容错解析）。
 */
static void ReadJsonBool(cJSON *root, const char *name, bool &out)
{
    cJSON *item = cJSON_GetObjectItem(root, name);
    if (item != nullptr && cJSON_IsBool(item)) {
        out = cJSON_IsTrue(item);
    }
}

/** 解析 "密文hex,noncehex" 加密串，还原密文（含 tag）与 nonce */
static bool ParseCipherText(const std::string &cipher, std::vector<uint8_t> &cipherVec, uint8_t *nonce)
{
    size_t pos = cipher.find(ENCRYT_SPLIT_SEP);
    if (pos == std::string::npos) {
        NETMGR_EXT_LOG_E("ParseCipherText: invalid cipher format");
        return false;
    }
    std::string cipherHex = cipher.substr(0, pos);
    std::string ivHex = cipher.substr(pos + 1);
    cipherVec.assign(cipherHex.length() / 2, 0);
    uint32_t length = 0;
    if (HexStringToVec(cipherHex, cipherVec.data(), cipherVec.size(), length) != 0) {
        NETMGR_EXT_LOG_E("ParseCipherText: cipher hex invalid");
        return false;
    }
    if (length < GCM_TAG_SIZE) {
        NETMGR_EXT_LOG_E("ParseCipherText: cipher too short");
        return false;
    }
    cipherVec.resize(length);
    uint32_t lengthIV = NONCE_SIZE;
    if (HexStringToVec(ivHex, nonce, NONCE_SIZE, lengthIV) != 0 || lengthIV != NONCE_SIZE) {
        NETMGR_EXT_LOG_E("ParseCipherText: nonce hex invalid");
        return false;
    }
    return true;
}

/** 按别名获取 HUKS 密钥，不存在则先生成 */
static int32_t EthEapGetKeyByAlias(struct HksBlob *keyAlias, const struct HksParamSet *genParamSet)
{
    if (keyAlias == nullptr || genParamSet == nullptr) {
        return -1;
    }
    int32_t keyExist = HksKeyExist(keyAlias, genParamSet);
    if (keyExist == HKS_ERROR_NOT_EXIST) {
        return HksGenerateKey(keyAlias, genParamSet, nullptr);
    }
    return keyExist;
}

/** 在通用参数集基础上追加本次随机 nonce，构建加解密参数集 */
static int32_t EthEapBuildHksParamSet(struct HksParamSet **paramSet, uint8_t *nonce, uint32_t nonceSize)
{
    struct HksParam IVParam[] = {
        { .tag = HKS_TAG_NONCE, .blob = { .size = nonceSize, .data = nonce } },
    };
    int32_t ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("HksInitParamSet failed");
        return ret;
    }
    ret = HksAddParams(*paramSet, g_ethEapGenParam, sizeof(g_ethEapGenParam) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(paramSet);
        return ret;
    }
    ret = HksAddParams(*paramSet, IVParam, sizeof(IVParam) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(paramSet);
        return ret;
    }
    ret = HksBuildParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(paramSet);
        return ret;
    }
    return ret;
}

EthEapConfigStore &EthEapConfigStore::GetInstance()
{
    // 静态局部变量的初始化本身线程安全，可直接用作单例
    static EthEapConfigStore instance;
    return instance;
}

EthEapConfigStore::EthEapConfigStore()
{
    int32_t ret = HksInitialize();
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("EthEap HUKS init failed: %{public}d", ret);
    } else {
        hksInitialized_ = true;
    }
}

bool EthEapConfigStore::EnsureDirExists()
{
    struct stat st;
    if (stat(ETH_EAP_CONFIG_DIR, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir(ETH_EAP_CONFIG_DIR, S_IRWXU | S_IRWXG) == 0) {
        return true;
    }
    NETMGR_EXT_LOG_E("Failed to create dir: %{public}s", ETH_EAP_CONFIG_DIR);
    return false;
}

int32_t EthEapConfigStore::EncryptField(const std::string &plain, std::string &cipher)
{
    if (plain.empty()) {
        cipher = "";
        return NETMANAGER_EXT_SUCCESS;
    }
    std::string fileName = std::string(ETH_EAP_ENCRY_KEY) + ETH_EAP_ENCRY_FILE;
    struct HksBlob keyAlias = { fileName.length(), (uint8_t *)&fileName[0] };
    struct HksBlob plainText = { plain.length(), (uint8_t *)&plain[0] };

    // 每次加密生成新随机 nonce，防重放
    uint8_t nonce[NONCE_SIZE] = {0};
    struct HksBlob randomIV = { NONCE_SIZE, nonce };
    int32_t ret = HksGenerateRandom(nullptr, &randomIV);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("EthEap generate IV failed: %{public}d", ret);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    struct HksParamSet *encryParamSet = nullptr;
    ret = EthEapBuildHksParamSet(&encryParamSet, nonce, NONCE_SIZE);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("EthEap build paramset failed: %{public}d", ret);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    ret = EthEapGetKeyByAlias(&keyAlias, encryParamSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("EthEap get key failed: %{public}d", ret);
        HksFreeParamSet(&encryParamSet);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    // AES-256-GCM 加密（输出=明文+tag），缓冲区按明文长度动态分配，支持长证书字段
    uint32_t cipherLen = static_cast<uint32_t>(plain.length()) + GCM_TAG_SIZE;
    std::vector<uint8_t> cipherBuf(cipherLen, 0);
    struct HksBlob cipherData = { cipherLen, cipherBuf.data() };
    ret = HksEncrypt(&keyAlias, encryParamSet, &plainText, &cipherData);
    HksFreeParamSet(&encryParamSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("EthEap HksEncrypt failed: %{public}d", ret);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    // 输出格式 "密文hex,noncehex"（解密时必须使用同一 nonce）
    cipher = ConvertArrayToHex(cipherBuf.data(), cipherData.size) + ENCRYT_SPLIT_SEP +
             ConvertArrayToHex(nonce, NONCE_SIZE);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthEapConfigStore::DecryptField(const std::string &cipher, std::string &plain)
{
    if (cipher.empty()) {
        plain = "";
        return NETMANAGER_EXT_SUCCESS;
    }
    std::string fileName = std::string(ETH_EAP_ENCRY_KEY) + ETH_EAP_ENCRY_FILE;
    struct HksBlob keyAlias = { fileName.length(), (uint8_t *)&fileName[0] };

    // 解析 "密文hex,noncehex" 格式，还原二进制密文与 nonce
    uint8_t nonce[NONCE_SIZE] = {0};
    std::vector<uint8_t> cipherVec;
    if (!ParseCipherText(cipher, cipherVec, nonce)) {
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    struct HksBlob cipherData = { static_cast<uint32_t>(cipherVec.size()), cipherVec.data() };
    struct HksParamSet *decryParamSet = nullptr;
    int32_t ret = EthEapBuildHksParamSet(&decryParamSet, nonce, NONCE_SIZE);
    if (ret != HKS_SUCCESS) {
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    // 密钥丢失则无法解密，属不可恢复错误
    ret = HksKeyExist(&keyAlias, decryParamSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("EthEap decrypt: key not exist");
        HksFreeParamSet(&decryParamSet);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    // GCM 自动校验 tag（篡改/密钥不符失败）；实际明文长度由 plainText.size 回传
    std::vector<uint8_t> plainBuf(cipherVec.size(), 0);
    HksBlob plainText = { static_cast<uint32_t>(plainBuf.size()), plainBuf.data() };
    ret = HksDecrypt(&keyAlias, decryParamSet, &cipherData, &plainText);
    HksFreeParamSet(&decryParamSet);
    if (ret != HKS_SUCCESS) {
        NETMGR_EXT_LOG_E("EthEap HksDecrypt failed: %{public}d", ret);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    plain.assign(reinterpret_cast<char *>(plainText.data), plainText.size);
    return NETMANAGER_EXT_SUCCESS;
}

/**
 * 敏感字段序列化辅助：明文加密写入 JSON 后立即清零栈上副本；
 * 加密失败使整体序列化失败，不允许静默丢字段。
 */
bool EthEapConfigStore::AddEncryptedField(cJSON *root, const char *name, const char *plainData, size_t plainLen)
{
    std::string plain(plainData, plainLen);
    std::string cipher;
    int32_t ret = EncryptField(plain, cipher);
    (void)memset_s(plain.data(), plain.size(), 0, plain.size());
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("AddEncryptedField: encrypt %{public}s failed, ret: %{public}d", name, ret);
        return false;
    }
    cJSON_AddStringToObject(root, name, cipher.c_str());
    return true;
}

/** 敏感字段反序列化辅助：解密还原到 SecureData 后清零临时副本，缺失/失败保持原值 */
void EthEapConfigStore::ReadEncryptedField(cJSON *root, const char *name, SecureData &out)
{
    cJSON *item = cJSON_GetObjectItem(root, name);
    if (item == nullptr || !cJSON_IsString(item)) {
        return;
    }
    std::string plain;
    if (DecryptField(item->valuestring, plain) == NETMANAGER_EXT_SUCCESS) {
        out.append(plain.c_str(), plain.size());
    }
    (void)memset_s(plain.data(), plain.size(), 0, plain.size());
}

/** 敏感二进制字段反序列化辅助：解密还原到字节数组后清零临时副本 */
void EthEapConfigStore::ReadEncryptedBytesField(cJSON *root, const char *name, std::vector<uint8_t> &out)
{
    cJSON *item = cJSON_GetObjectItem(root, name);
    if (item == nullptr || !cJSON_IsString(item)) {
        return;
    }
    std::string plain;
    if (DecryptField(item->valuestring, plain) == NETMANAGER_EXT_SUCCESS) {
        out.assign(plain.begin(), plain.end());
    }
    (void)memset_s(plain.data(), plain.size(), 0, plain.size());
}

std::string EthEapConfigStore::SerializeConfig(const EthEapConfig &config)
{
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        return "";
    }
    const EthEapProfile &p = config.profile;
    cJSON_AddNumberToObject(root, "eapMethod", static_cast<int>(p.eapMethod));
    cJSON_AddNumberToObject(root, "phase2Method", static_cast<int>(p.phase2Method));
    cJSON_AddStringToObject(root, "identity", p.identity.c_str());
    cJSON_AddStringToObject(root, "anonymousIdentity", p.anonymousIdentity.c_str());

    bool encOk = true;
    encOk = AddEncryptedField(root, "password", p.password.c_str(), p.password.size()) && encOk;

    cJSON_AddStringToObject(root, "caCertAliases", p.caCertAliases.c_str());
    cJSON_AddStringToObject(root, "caPath", p.caPath.c_str());
    cJSON_AddStringToObject(root, "clientCertAliases", p.clientCertAliases.c_str());

    encOk = AddEncryptedField(root, "certEntry", reinterpret_cast<const char *>(p.certEntry.data()),
        p.certEntry.size()) && encOk;
    encOk = AddEncryptedField(root, "certPassword", p.certPassword.c_str(), p.certPassword.size()) && encOk;
    if (!encOk) {
        NETMGR_EXT_LOG_E("SerializeConfig: encrypt sensitive field failed");
        cJSON_Delete(root);
        return "";
    }

    cJSON_AddStringToObject(root, "altSubjectMatch", p.altSubjectMatch.c_str());
    cJSON_AddStringToObject(root, "domainSuffixMatch", p.domainSuffixMatch.c_str());
    cJSON_AddStringToObject(root, "realm", p.realm.c_str());
    cJSON_AddStringToObject(root, "plmn", p.plmn.c_str());
    cJSON_AddNumberToObject(root, "eapSubId", p.eapSubId);
    cJSON_AddBoolToObject(root, "enabled", config.enabled);
    cJSON_AddBoolToObject(root, "autoAuth", config.autoAuth);
    cJSON_AddBoolToObject(root, "locked", config.locked);

    char *jsonStr = cJSON_Print(root);
    cJSON_Delete(root);
    if (jsonStr == nullptr) {
        return "";
    }
    std::string result(jsonStr);
    free(jsonStr);
    return result;
}

bool EthEapConfigStore::DeserializeConfig(const std::string &jsonStr, EthEapConfig &config)
{
    cJSON *root = cJSON_Parse(jsonStr.c_str());
    if (root == nullptr) {
        NETMGR_EXT_LOG_E("DeserializeConfig: cJSON_Parse failed");
        return false;
    }
    EthEapProfile &p = config.profile;
    int32_t eapMethod = static_cast<int32_t>(p.eapMethod);
    ReadJsonNumber(root, "eapMethod", eapMethod);
    p.eapMethod = static_cast<EapMethod>(eapMethod);
    int32_t phase2Method = static_cast<int32_t>(p.phase2Method);
    ReadJsonNumber(root, "phase2Method", phase2Method);
    p.phase2Method = static_cast<Phase2Method>(phase2Method);
    ReadJsonString(root, "identity", p.identity);
    ReadJsonString(root, "anonymousIdentity", p.anonymousIdentity);
    ReadEncryptedField(root, "password", p.password);
    ReadJsonString(root, "caCertAliases", p.caCertAliases);
    ReadJsonString(root, "caPath", p.caPath);
    ReadJsonString(root, "clientCertAliases", p.clientCertAliases);
    ReadEncryptedBytesField(root, "certEntry", p.certEntry);
    ReadEncryptedField(root, "certPassword", p.certPassword);
    ReadJsonString(root, "altSubjectMatch", p.altSubjectMatch);
    ReadJsonString(root, "domainSuffixMatch", p.domainSuffixMatch);
    ReadJsonString(root, "realm", p.realm);
    ReadJsonString(root, "plmn", p.plmn);
    ReadJsonNumber(root, "eapSubId", p.eapSubId);
    ReadJsonBool(root, "enabled", config.enabled);
    ReadJsonBool(root, "autoAuth", config.autoAuth);
    ReadJsonBool(root, "locked", config.locked);
    cJSON_Delete(root);
    return true;
}

/** 逐字段比较两份配置是否完全一致（用于内容未变化时跳过落盘） */
static bool IsSameConfig(const EthEapConfig &a, const EthEapConfig &b)
{
    const EthEapProfile &pa = a.profile;
    const EthEapProfile &pb = b.profile;
    return pa.eapMethod == pb.eapMethod && pa.phase2Method == pb.phase2Method && pa.identity == pb.identity &&
           pa.anonymousIdentity == pb.anonymousIdentity && pa.password == pb.password &&
           pa.caCertAliases == pb.caCertAliases && pa.caPath == pb.caPath &&
           pa.clientCertAliases == pb.clientCertAliases && pa.certEntry == pb.certEntry &&
           pa.certPassword == pb.certPassword && pa.altSubjectMatch == pb.altSubjectMatch &&
           pa.domainSuffixMatch == pb.domainSuffixMatch && pa.realm == pb.realm && pa.plmn == pb.plmn &&
           pa.eapSubId == pb.eapSubId && a.enabled == b.enabled && a.autoAuth == b.autoAuth && a.locked == b.locked;
}

int32_t EthEapConfigStore::SaveConfig(const EthEapConfig &config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!EnsureDirExists()) {
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    // 与已落盘内容一致时跳过写入，减少 flash 磨损；密文含随机 nonce，
    // 须反序列化旧文件按字段比较，不能直接比较序列化串
    struct stat st;
    if (stat(ETH_EAP_CONFIG_PATH, &st) == 0 && S_ISREG(st.st_mode)) {
        std::ifstream existFile(ETH_EAP_CONFIG_PATH, std::ios::in);
        if (existFile.is_open()) {
            std::stringstream ss;
            ss << existFile.rdbuf();
            existFile.close();
            EthEapConfig existConfig;
            if (!ss.str().empty() && DeserializeConfig(ss.str(), existConfig) && IsSameConfig(existConfig, config)) {
                NETMGR_EXT_LOG_I("SaveConfig: config unchanged, skip saving");
                return NETMANAGER_EXT_SUCCESS;
            }
        }
    }
    std::string jsonStr = SerializeConfig(config);
    if (jsonStr.empty()) {
        NETMGR_EXT_LOG_E("SaveConfig: serialize failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    std::ofstream file(ETH_EAP_CONFIG_PATH, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        NETMGR_EXT_LOG_E("SaveConfig: open file failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    file << jsonStr;
    file.close();
    // 仅属主可读写，防其他进程读取
    chmod(ETH_EAP_CONFIG_PATH, S_IRUSR | S_IWUSR);
    NETMGR_EXT_LOG_I("SaveConfig success");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthEapConfigStore::LoadConfig(EthEapConfig &config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    struct stat st;
    // 文件不存在返回专用错误码，调用方区分无配置与读取失败
    // lstat：符号链接返回链接自身 mode
    if (lstat(ETH_EAP_CONFIG_PATH, &st) != 0) {
        NETMGR_EXT_LOG_D("LoadConfig: config file not exist");
        return NETMANAGER_EXT_ERR_FILE_NOT_FOUND;
    }
    // 拒绝符号链接防止被替换指向其他文件（必须 lstat 才有效）
    if (S_ISLNK(st.st_mode)) {
        NETMGR_EXT_LOG_E("LoadConfig: config file is symlink, rejected");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    std::ifstream file(ETH_EAP_CONFIG_PATH, std::ios::in);
    if (!file.is_open()) {
        NETMGR_EXT_LOG_E("LoadConfig: open file failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    std::string jsonStr = ss.str();
    if (!DeserializeConfig(jsonStr, config)) {
        NETMGR_EXT_LOG_E("LoadConfig: deserialize failed");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthEapConfigStore::DeleteConfig()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // ENOENT 不算错误（幂等）
    if (unlink(ETH_EAP_CONFIG_PATH) != 0 && errno != ENOENT) {
        NETMGR_EXT_LOG_E("DeleteConfig: unlink failed, errno: %{public}d", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    // 销毁 HUKS 密钥（幂等，失败仅记日志不影响删除结果）
    std::string fileName = std::string(ETH_EAP_ENCRY_KEY) + ETH_EAP_ENCRY_FILE;
    struct HksBlob keyAlias = { fileName.length(), (uint8_t *)&fileName[0] };
    uint8_t nonce[NONCE_SIZE] = {0};
    struct HksParamSet *delParamSet = nullptr;
    if (EthEapBuildHksParamSet(&delParamSet, nonce, NONCE_SIZE) == HKS_SUCCESS) {
        int32_t keyRet = HksDeleteKey(&keyAlias, delParamSet);
        if (keyRet != HKS_SUCCESS && keyRet != HKS_ERROR_NOT_EXIST) {
            NETMGR_EXT_LOG_E("DeleteConfig: HksDeleteKey failed: %{public}d", keyRet);
        }
        HksFreeParamSet(&delParamSet);
    } else {
        NETMGR_EXT_LOG_E("DeleteConfig: build paramset for HksDeleteKey failed");
    }
    NETMGR_EXT_LOG_I("DeleteConfig success");
    return NETMANAGER_EXT_SUCCESS;
}

bool EthEapConfigStore::HasConfig()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 符号链接不算有配置（lstat）
    struct stat st;
    return lstat(ETH_EAP_CONFIG_PATH, &st) == 0 && S_ISREG(st.st_mode);
}

} // namespace NetManagerStandard
} // namespace OHOS
