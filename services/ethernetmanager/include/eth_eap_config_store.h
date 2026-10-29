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

#ifndef ETH_EAP_CONFIG_STORE_H
#define ETH_EAP_CONFIG_STORE_H

#include <string>
#include <mutex>
#include <vector>
#include "eth_eap_config.h"

typedef struct cJSON cJSON;

namespace OHOS {
namespace NetManagerStandard {

class EthEapConfigStore {
public:
    static EthEapConfigStore &GetInstance();

    int32_t SaveConfig(const EthEapConfig &config);
    int32_t LoadConfig(EthEapConfig &config);
    int32_t DeleteConfig();
    bool HasConfig();

private:
    EthEapConfigStore();
    ~EthEapConfigStore() = default;

    int32_t EncryptField(const std::string &plain, std::string &cipher);
    int32_t DecryptField(const std::string &cipher, std::string &plain);
    bool AddEncryptedField(cJSON *root, const char *name, const char *plainData, size_t plainLen);
    void ReadEncryptedField(cJSON *root, const char *name, SecureData &out);
    void ReadEncryptedBytesField(cJSON *root, const char *name, std::vector<uint8_t> &out);
    std::string SerializeConfig(const EthEapConfig &config);
    bool DeserializeConfig(const std::string &jsonStr, EthEapConfig &config);
    bool EnsureDirExists();

    std::mutex mutex_;
    bool hksInitialized_ = false;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_CONFIG_STORE_H
