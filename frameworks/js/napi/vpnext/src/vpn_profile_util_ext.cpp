/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "vpn_profile_util_ext.h"
#include "preferences_helper.h"
#include "netmanager_ext_log.h"
#include "ability_manager_client.h"
#include "application_context.h"

namespace OHOS {
namespace NetManagerStandard {
VpnProfileUtilExt::VpnProfileUtilExt() {
}
VpnProfileUtilExt::~VpnProfileUtilExt() {}

std::shared_ptr<NativePreferences::Preferences> VpnProfileUtilExt::GetProfiles(const std::string &path, int &errCode)
{
    std::shared_ptr<NativePreferences::Preferences> ptr =
        NativePreferences::PreferencesHelper::GetPreferences(path, errCode);
    if (ptr == nullptr) {
        NETMANAGER_EXT_LOGI("GetPreference error code is %{public}d", errCode);
    }
    return ptr;
}

int VpnProfileUtilExt::DeleteProfiles()
{
    return NativePreferences::PreferencesHelper::DeletePreferences(path_);
}

int VpnProfileUtilExt::SaveString(const std::string &key, const std::string &value)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    int ret = ptr->PutString(key, value);
    ptr->Flush();
    return ret;
}

std::string VpnProfileUtilExt::ObtainString(const std::string &key, const std::string &defValue)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return error_;
    }
    return ptr->GetString(key, defValue);
}

int VpnProfileUtilExt::SaveInt(const std::string &key, int value)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    int ret = ptr->PutInt(key, value);
    ptr->Flush();
    return ret;
}

int VpnProfileUtilExt::ObtainInt(const std::string &key, int defValue)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    return ptr->GetInt(key, defValue);
}

int VpnProfileUtilExt::SaveBool(const std::string &key, bool value)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    int ret = ptr->PutBool(key, value);
    ptr->Flush();
    return ret;
}

bool VpnProfileUtilExt::ObtainBool(const std::string &key, bool defValue)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        NETMANAGER_EXT_LOGI("ObtainBool path if nullptr %{public}s", path_.c_str());
        return NativePreferences::E_ERROR;
    }
    bool result = ptr->GetBool(key, defValue);
    return result;
}

int VpnProfileUtilExt::SaveLong(const std::string &key, int64_t value)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    int ret = ptr->PutLong(key, value);
    ptr->Flush();
    return ret;
}

int64_t VpnProfileUtilExt::ObtainLong(const std::string &key, int64_t defValue)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    return ptr->GetLong(key, defValue);
}

int VpnProfileUtilExt::SaveFloat(const std::string &key, float value)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    int ret = ptr->PutFloat(key, value);
    ptr->Flush();
    return ret;
}

float VpnProfileUtilExt::ObtainFloat(const std::string &key, float defValue)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    return ptr->GetFloat(key, defValue);
}

bool VpnProfileUtilExt::IsExistKey(const std::string &key)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        NETMANAGER_EXT_LOGI("IsExistKey path if nullptr %{public}s", path_.c_str());
        return NativePreferences::E_ERROR;
    }
    bool result = ptr->HasKey(key);
    return result;
}

int VpnProfileUtilExt::RemoveKey(const std::string &key)
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    return ptr->Delete(key);
}

int VpnProfileUtilExt::RemoveAll()
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    return ptr->Clear();
}

void VpnProfileUtilExt::Refresh()
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr != nullptr) {
        ptr->Flush();
    }
}

int VpnProfileUtilExt::RefreshSync()
{
    std::shared_ptr<NativePreferences::Preferences> ptr = GetProfiles(path_, errCode_);
    if (ptr == nullptr) {
        return NativePreferences::E_ERROR;
    }
    return ptr->FlushSync();
}
} // namespace NetManagerStandard
} // namespace OHOS