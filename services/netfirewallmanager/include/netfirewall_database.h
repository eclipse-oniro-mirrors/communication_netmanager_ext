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

#ifndef FIREWALL_DATABASE_H
#define FIREWALL_DATABASE_H

#include "rdb_common.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"
#include "system_ability.h"
#include "value_object.h"
#include <string>

namespace OHOS {
namespace NetManagerStandard {
static std::string FIREWALL_DB_PATH = "/data/service/el1/public/netmanager/";

constexpr const char *FIREWALL_DB_NAME = "netfirewall.db";
constexpr const char *FIREWALL_TABLE_NAME = "firewallRule";
constexpr const char *FIREWALL_TABLE_IP_RULE = "firewallIpRule";
constexpr const char *FIREWALL_TABLE_PORT_RULE = "firewallPortRule";
constexpr const char *FIREWALL_TABLE_DOMAIN_RULE = "firewallDomainRule";
constexpr const char *INTERCEPT_RECORD_TABLE = "interceptRecord";
constexpr int32_t DATABASE_OPEN_VERSION = 1;
constexpr int32_t DATABASE_NEW_VERSION = 2;

constexpr const char *CREATE_FIREWALL_TABLE = "CREATE TABLE IF NOT EXISTS [firewallRule]("
    "[ruleId] INTEGER PRIMARY KEY, "
    "[ruleName] TEXT NOT NULL, "
    "[ruleDescription] TEXT, "
    "[userId] INTEGER NOT NULL, "
    "[ruleDirection] INTEGER NOT NULL, "
    "[ruleAction] INTEGER NOT NULL, "
    "[ruleType] INTEGER NOT NULL, "
    "[isEnabled] INTEGER NOT NULL, "
    "[appUid] INTEGER, "
    "[protocol] INTEGER, "
    "[primaryDns] TEXT, "
    "[standbyDns] TEXT );";

constexpr const char *CREATE_FIREWALL_IP_RULE_TABLE = "CREATE TABLE IF NOT EXISTS [firewallIpRule]("
    "[id] INTEGER PRIMARY KEY, "
    "[ruleId] INTEGER NOT NULL, "
    "[userId] INTEGER NOT NULL, "
    "[appUid] INTEGER, "
    "[locationType] INTEGER NOT NULL, "
    "[family] INTEGER, "
    "[type] INTEGER, "
    "[address] TEXT, "
    "[mask] INTEGER, "
    "[startIp] TEXT, "
    "[endIp] TEXT);";

constexpr const char *CREATE_FIREWALL_PORT_RULE_TABLE = "CREATE TABLE IF NOT EXISTS [firewallPortRule]("
    "[id] INTEGER PRIMARY KEY, "
    "[ruleId] INTEGER NOT NULL, "
    "[userId] INTEGER NOT NULL, "
    "[appUid] INTEGER, "
    "[locationType] INTEGER NOT NULL, "
    "[startPort] INTEGER, "
    "[endPort] INTEGER);";

constexpr const char *CREATE_FIREWALL_DOMAIN_RULE_TABLE = "CREATE TABLE IF NOT EXISTS [firewallDomainRule]("
    "[id] INTEGER PRIMARY KEY, "
    "[ruleId] INTEGER NOT NULL, "
    "[userId] INTEGER NOT NULL, "
    "[appUid] INTEGER, "
    "[isWildcard] INTEGER, "
    "[domain] TEXT);";

constexpr const char *CREATE_RECORD_TABLE = "CREATE TABLE IF NOT EXISTS [interceptRecord]("
    "[id] INTEGER PRIMARY KEY, "
    "[userId] INTEGER NOT NULL, "
    "[time] INTEGER NOT NULL, "
    "[sourceIp] TEXT, "
    "[destIp] TEXT, "
    "[sourcePort] INTEGER, "
    "[destPort] INTEGER, "
    "[protocol] INTEGER, "
    "[appUid] INTEGER, "
    "[domain] TEXT);";

class NetFirewallDataBase : public NoCopyable {
public:
    static std::shared_ptr<NetFirewallDataBase> GetInstance();

    int64_t Insert(const OHOS::NativeRdb::ValuesBucket &insertValues, const std::string tableName);

    int32_t Update(const std::string &tableName, int32_t &changedRows, const OHOS::NativeRdb::ValuesBucket &values,
        const std::string &whereClause, const std::vector<std::string> &whereArgs);

    int32_t Delete(const std::string &tableName, int32_t &changedRows, const std::string &whereClause,
        const std::vector<std::string> &whereArgs);

    std::shared_ptr<OHOS::NativeRdb::ResultSet> Query(const OHOS::NativeRdb::AbsRdbPredicates &predicates,
        const std::vector<std::string> &columns);

    int32_t BeginTransaction();

    int32_t Commit();

    int32_t RollBack();

    int32_t Count(int64_t &outValue, const OHOS::NativeRdb::AbsRdbPredicates &predicates);

private:
    NetFirewallDataBase();

    static std::shared_ptr<NetFirewallDataBase> instance_;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
};

class NetFirewallDataBaseCallBack : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;

    int32_t OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;

    int32_t OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int32_t currentVersion, int32_t targetVersion) override;

private:
    std::map<std::string, std::string> netfirewallTableMap_;
    void initNetFirewallTableMap();
};

enum FirewallDBErrCode {
    FIREWALL_OK = 0,
    FIREWALL_FAILURE = -1,
    FIREWALL_RDB_EXECUTE_FAILTURE = -2,
    FIREWALL_RDB_NO_INIT = -3,
    FIREWALL_RDB_EMPTY = -4,
    FIREWALL_PERMISSION_DENIED = -5,
    FIREWALL_NOP = -6,
    FIREWALL_OVERFLOW = -7,
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // FIREWALL_DATABASE_H