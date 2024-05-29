/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/stat.h>
#include <sys/types.h>

#include "netfirewall_preferences_util.h"
#include "netmgr_ext_log_wrapper.h"
#include "preferences_errno.h"
#include "preferences_helper.h"
#include "preferences_value.h"
using namespace std;

namespace OHOS {
namespace NetManagerStandard {
shared_ptr<NetFirewallPreferencesUtil> NetFirewallPreferencesUtil::GetInstance()
{
    static std::shared_ptr<NetFirewallPreferencesUtil> instance = make_shared<NetFirewallPreferencesUtil>();
    return instance;
}

bool NetFirewallPreferencesUtil::GetPreference()
{
    if (ptr_ != nullptr) {
        return true;
    }
    int32_t errCode = -1;
    ptr_ = NativePreferences::PreferencesHelper::GetPreferences(filePath_, errCode);
    if (ptr_ == nullptr) {
        NETMGR_EXT_LOG_E("GetPreference error code is %{public}d", errCode);
        return false;
    }
    return true;
}

bool NetFirewallPreferencesUtil::GetPreference(const std::string &filePath)
{
    if (ptr_ != nullptr) {
        return true;
    }
    filePath_ = filePath;
    int32_t errCode = -1;
    ptr_ = NativePreferences::PreferencesHelper::GetPreferences(filePath_, errCode);
    if (ptr_ == nullptr) {
        NETMGR_EXT_LOG_E("GetPreference error code is %{public}d", errCode);
        return false;
    }
    return true;
}

bool NetFirewallPreferencesUtil::SaveInt(const std::string &key, int32_t value)
{
    return Save(key, value);
}

bool NetFirewallPreferencesUtil::SaveBool(const std::string &key, bool value)
{
    return Save(key, value);
}

template <typename T> bool NetFirewallPreferencesUtil::Save(const std::string &key, const T &value)
{
    if (!GetPreference()) {
        return false;
    }
    if (!SaveInner(ptr_, key, value)) {
        NETMGR_EXT_LOG_E("SaveInner error");
        return false;
    }
    return RefreshSync();
}

bool NetFirewallPreferencesUtil::SaveInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key,
    const int32_t &value)
{
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is nullptr");
        return false;
    }
    return ptr->PutInt(key, value) == NativePreferences::E_OK;
}

bool NetFirewallPreferencesUtil::SaveInner(std::shared_ptr<NativePreferences::Preferences> ptr, const std::string &key,
    const bool &value)
{
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr is nullptr");
        return false;
    }
    return ptr->PutBool(key, value) == NativePreferences::E_OK;
}

int32_t NetFirewallPreferencesUtil::ObtainInt(const std::string &key, int32_t defValue)
{
    return Obtain(key, defValue);
}

bool NetFirewallPreferencesUtil::ObtainBool(const std::string &key, bool defValue)
{
    return Obtain(key, defValue);
}

template <typename T> T NetFirewallPreferencesUtil::Obtain(const std::string &key, const T &defValue)
{
    if (!GetPreference()) {
        NETMGR_EXT_LOG_I("Obtain GetPreference failed");
        return defValue;
    }
    return ObtainInner(ptr_, key, defValue);
}

int32_t NetFirewallPreferencesUtil::ObtainInner(std::shared_ptr<NativePreferences::Preferences> ptr,
    const std::string &key, const int32_t &defValue)
{
    return ptr->GetInt(key, defValue);
}

bool NetFirewallPreferencesUtil::ObtainInner(std::shared_ptr<NativePreferences::Preferences> ptr,
    const std::string &key, const bool &defValue)
{
    return ptr->GetBool(key, defValue);
}

bool NetFirewallPreferencesUtil::RefreshSync()
{
    if (!GetPreference()) {
        NETMGR_EXT_LOG_I("RefreshSync GetPreference failed");
        return false;
    }
    if (ptr_ == nullptr) {
        NETMGR_EXT_LOG_E("ptr_ is nullptr");
        return false;
    }
    if (ptr_->FlushSync() != NativePreferences::E_OK) {
        NETMGR_EXT_LOG_E("RefreshSync error");
        return false;
    }
    return true;
}

bool NetFirewallPreferencesUtil::Clear(const std::string &filePath)
{
    if (!GetPreference(filePath)) {
        NETMGR_EXT_LOG_I("Clear GetPreference failed");
        return false;
    }
    if (ptr_ == nullptr) {
        NETMGR_EXT_LOG_E("ptr_ is nullptr");
        return false;
    }
    int ret = NativePreferences::PreferencesHelper::DeletePreferences(filePath);
    if (ret) {
        NETMGR_EXT_LOG_E("ptr_ is nullptr");
    }
    ptr_ = nullptr;
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS