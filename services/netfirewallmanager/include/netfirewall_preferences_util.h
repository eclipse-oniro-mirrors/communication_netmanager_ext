/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NET_FIREWALL_PREFERENCES_UTIL_H
#define NET_FIREWALL_PREFERENCES_UTIL_H

#include <map>

#include "preferences.h"
namespace OHOS {
namespace NetManagerStandard {
class NetFirewallPreferencesUtil {
public:
    static std::shared_ptr<NetFirewallPreferencesUtil> GetInstance();
    NetFirewallPreferencesUtil() = default;
    ~NetFirewallPreferencesUtil() = default;

    bool SaveInt(const std::string &key, int32_t value);
    bool SaveBool(const std::string &key, bool value);
    bool SaveString(const std::string &key, std::string value);

    int32_t ObtainInt(const std::string &key, int32_t defValue);
    bool ObtainBool(const std::string &key, bool defValue);
    std::string ObtainString(const std::string &key, std::string defValue);
    std::map<std::string, NativePreferences::PreferencesValue> ObtainAll();

    bool IsExist(const std::string &key);
    bool Remove(const std::string &key);
    bool Clear(const std::string &filePath);
    bool GetPreference(const std::string &filePath);

private:
    bool GetPreference();
    bool RefreshSync();

    template <typename T> bool Save(const std::string &key, const T &defValue);

    bool SaveInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key, const int32_t &value);
    bool SaveInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key, const bool &value);
    bool SaveInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key,
                   const std::string &value);

    template <typename T> T Obtain(const std::string &key, const T &defValue);

    int32_t ObtainInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key,
                        const int32_t &defValue);
    bool ObtainInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key, const bool &defValue);
    std::string ObtainInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key,
                            const std::string &defValue);
    std::shared_ptr<NativePreferences::Preferences> ptr_ = nullptr;
    std::string filePath_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_PREFERENCES_UTIL_H