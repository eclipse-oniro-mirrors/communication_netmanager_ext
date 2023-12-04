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

#include "net_datashare_utils.h"

#include <vector>

#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *SETTINGS_DATASHARE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr const char *SETTINGS_DATA_COLUMN_KEYWORD = "KEYWORD";
constexpr const char *SETTINGS_DATA_COLUMN_VALUE = "VALUE";

constexpr int32_t INVALID_VALUE = -1;
} // namespace

NetDataShareHelperUtils::NetDataShareHelperUtils()
{
    dataShareHelper_ = CreateDataShareHelper();
}

std::shared_ptr<DataShare::DataShareHelper> NetDataShareHelperUtils::CreateDataShareHelper()
{
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        NETMGR_LOG_E("NetDataShareHelperUtils GetSystemAbilityManager failed.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        NETMGR_LOG_E("NetDataShareHelperUtils GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, SETTINGS_DATASHARE_URI, SETTINGS_DATA_EXT_URI);
}

int32_t NetDataShareHelperUtils::Query(Uri &uri, const std::string &key, std::string &value)
{
    if (dataShareHelper_ == nullptr) {
        NETMGR_LOG_E("dataShareHelper_ is nullptr");
        return NETMANAGER_ERROR;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    predicates.EqualTo(SETTINGS_DATA_COLUMN_KEYWORD, key);
    auto result = dataShareHelper_->Query(uri, predicates, columns);
    if (result == nullptr) {
        NETMGR_LOG_E("DataShareHelper query error, result is null");
        return NETMANAGER_ERROR;
    }

    if (result->GoToFirstRow() != DataShare::E_OK) {
        NETMGR_LOG_E("DataShareHelper query failed,go to first row error");
        result->Close();
        return NETMANAGER_ERROR;
    }

    int32_t columnIndex;
    result->GetColumnIndex(SETTINGS_DATA_COLUMN_VALUE, columnIndex);
    result->GetString(columnIndex, value);
    result->Close();
    NETMGR_LOG_I("DataShareHelper query success,value[%{public}s]", value.c_str());
    return NETMANAGER_SUCCESS;
}

int32_t NetDataShareHelperUtils::Insert(Uri &uri, const std::string &key, const std::string &value)
{
    if (dataShareHelper_ == nullptr) {
        NETMGR_LOG_E("dataShareHelper_ is nullptr");
        return NETMANAGER_ERROR;
    }
    DataShare::DataShareValuesBucket valuesBucket;
    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    valuesBucket.Put(SETTINGS_DATA_COLUMN_KEYWORD, keyObj);
    valuesBucket.Put(SETTINGS_DATA_COLUMN_VALUE, valueObj);
    int32_t result = dataShareHelper_->Insert(uri, valuesBucket);
    if (result == INVALID_VALUE) {
        NETMGR_LOG_E("DataShareHelper insert failed, insert result:%{public}d", result);
        return NETMANAGER_ERROR;
    }
    dataShareHelper_->NotifyChange(uri);
    NETMGR_LOG_I("DataShareHelper insert success");
    return NETMANAGER_SUCCESS;
}

int32_t NetDataShareHelperUtils::Update(Uri &uri, const std::string &key, const std::string &value)
{
    if (dataShareHelper_ == nullptr) {
        NETMGR_LOG_E("dataShareHelper_ is nullptr");
        return NETMANAGER_ERROR;
    }
    std::string queryValue;
    int32_t ret = Query(uri, key, queryValue);
    if (ret == NETMANAGER_ERROR) {
        return Insert(uri, key, value);
    }

    DataShare::DataShareValuesBucket valuesBucket;
    DataShare::DataShareValueObject valueObj(value);
    valuesBucket.Put(SETTINGS_DATA_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTINGS_DATA_COLUMN_KEYWORD, key);
    int32_t result = dataShareHelper_->Update(uri, predicates, valuesBucket);
    if (result == INVALID_VALUE) {
        return NETMANAGER_ERROR;
    }
    dataShareHelper_->NotifyChange(uri);
    NETMGR_LOG_I("DataShareHelper update success");
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS