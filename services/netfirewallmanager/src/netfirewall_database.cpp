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

#include <filesystem>
#include <system_error>

#include "hilog/log.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_database.h"

using namespace OHOS::NativeRdb;

namespace OHOS {
namespace NetManagerStandard {
std::shared_ptr<NetFirewallDataBase> NetFirewallDataBase::instance_ = nullptr;

NetFirewallDataBase::NetFirewallDataBase()
{
    if (!std::filesystem::exists(FIREWALL_DB_PATH)) {
        std::error_code ec;
        if (std::filesystem::create_directories(FIREWALL_DB_PATH, ec)) {
            NETMGR_EXT_LOG_D("create_directories success :%{public}s", FIREWALL_DB_PATH.c_str());
        } else {
            NETMGR_EXT_LOG_E("create_directories error :%{public}s : %s", FIREWALL_DB_PATH.c_str(),
                ec.message().c_str());
        }
    }
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    store_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    if (errCode != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("GetRdbStore errCode :%{public}d", errCode);
    } else {
        NETMGR_EXT_LOG_D("GetRdbStore success :%{public}d", errCode);
    }
}

std::shared_ptr<NetFirewallDataBase> NetFirewallDataBase::GetInstance()
{
    if (instance_ == nullptr) {
        NETMGR_EXT_LOG_W("reset to new instance");
        instance_.reset(new NetFirewallDataBase());
        return instance_;
    }
    return instance_;
}

int32_t NetFirewallDataBase::BeginTransaction()
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("BeginTransaction store_ is nullptr");
        return FIREWALL_RDB_NO_INIT;
    }
    int32_t ret = store_->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("BeginTransaction fail :%{public}d", ret);
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDataBase::Commit()
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("Commit store_ is nullptr");
        return FIREWALL_RDB_NO_INIT;
    }
    int32_t ret = store_->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("Commit fail :%{public}d", ret);
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDataBase::RollBack()
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("RollBack store_ is nullptr");
        return FIREWALL_RDB_NO_INIT;
    }
    int32_t ret = store_->RollBack();
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("RollBack fail :%{public}d", ret);
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    return FIREWALL_OK;
}

int64_t NetFirewallDataBase::Insert(const OHOS::NativeRdb::ValuesBucket &insertValues, const std::string tableName)
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("Insert store_ is  nullptr");
        return FIREWALL_RDB_NO_INIT;
    }
    int64_t outRowId = 0;
    int32_t ret = store_->Insert(outRowId, tableName, insertValues);
    NETMGR_EXT_LOG_D("Insert id=%{public}" PRIu64 "", outRowId);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("Insert ret :%{public}d", ret);
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    return outRowId;
}


int32_t NetFirewallDataBase::Update(const std::string &tableName, int32_t &changedRows,
    const OHOS::NativeRdb::ValuesBucket &values, const std::string &whereClause,
    const std::vector<std::string> &whereArgs)
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("Update(whereClause) store_ is nullptr");
        return FIREWALL_RDB_NO_INIT;
    }
    int32_t ret = store_->Update(changedRows, tableName, values, whereClause, whereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("Update(whereClause) ret :%{public}d", ret);
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    return FIREWALL_OK;
}


int32_t NetFirewallDataBase::Delete(const std::string &tableName, int32_t &changedRows, const std::string &whereClause,
    const std::vector<std::string> &whereArgs)
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("Delete store_ is nullptr");
        return FIREWALL_RDB_NO_INIT;
    }
    int32_t ret = store_->Delete(changedRows, tableName, whereClause, whereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("Delete(whereClause) ret :%{public}d", ret);
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    return FIREWALL_OK;
}


std::shared_ptr<OHOS::NativeRdb::ResultSet> NetFirewallDataBase::Query(
    const OHOS::NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns)
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("Query(AbsRdbPredicates) store_ is nullptr");
        return nullptr;
    }
    return store_->Query(predicates, columns);
}

int32_t NetFirewallDataBase::Count(int64_t &outValue, const OHOS::NativeRdb::AbsRdbPredicates &predicates)
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("Count(AbsRdbPredicates) store_ is nullptr");
        return FIREWALL_RDB_NO_INIT;
    }
    return store_->Count(outValue, predicates);
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> NetFirewallDataBase::QuerySql(const std::string &sql,
    const std::vector<std::string> &selectionArgs)
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("QuerySql(AbsRdbPredicates) store_ is nullptr");
        return nullptr;
    }
    return store_->QuerySql(sql, selectionArgs);
}

void NetFirewallDataBaseCallBack::initNetFirewallTableMap()
{
    netfirewallTableMap_.insert(std::pair<std::string, std::string>(FIREWALL_TABLE_NAME, CREATE_FIREWALL_TABLE));
    netfirewallTableMap_.insert(std::pair<std::string, std::string>(INTERCEPT_RECORD_TABLE, CREATE_RECORD_TABLE));
}

int32_t NetFirewallDataBaseCallBack::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    initNetFirewallTableMap();
    for (const auto &pair : netfirewallTableMap_) {
        std::string sql = pair.second;
        int32_t ret = store.ExecuteSql(sql);
        if (ret != OHOS::NativeRdb::E_OK) {
            NETMGR_EXT_LOG_E("OnCreate %{public}s, failed: %{public}d", pair.first.c_str(), ret);
            return FIREWALL_RDB_EXECUTE_FAILTURE;
        }
        NETMGR_EXT_LOG_D("DB OnCreate Done %{public}s ", pair.first.c_str());
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDataBaseCallBack::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    NETMGR_EXT_LOG_D("DB OnUpgrade Enter");
    (void)store;
    (void)oldVersion;
    (void)newVersion;
    return FIREWALL_OK;
}

int32_t NetFirewallDataBaseCallBack::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int32_t oldVersion,
    int32_t newVersion)
{
    NETMGR_EXT_LOG_D("DB OnDowngrade Enter");
    (void)store;
    (void)oldVersion;
    (void)newVersion;
    return FIREWALL_OK;
}
} // namespace NetManagerStandard
} // namespace OHOS