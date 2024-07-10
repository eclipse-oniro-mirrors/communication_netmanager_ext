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

#ifndef FIREWALL_DB_HELPER_H
#define FIREWALL_DB_HELPER_H

#include <string>

#include "netfirewall_database.h"
#include "netfirewall_common.h"
#include "rdb_common.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"
#include "system_ability.h"
#include "value_object.h"

namespace OHOS {
namespace NetManagerStandard {
// The data index of NetFirewallRule
struct NetFirewallRuleInfo {
    int32_t rowCount;
    int32_t ruleIdIndex;
    int32_t ruleNameIndex;
    int32_t ruleDescriptionIndex;
    int32_t ruleDirectionIndex;
    int32_t ruleActionIndex;
    int32_t ruleTypeIndex;
    int32_t isEnabledIndex;
    int32_t appUidIndex;
    int32_t protocolIndex;
    int32_t primaryDnsIndex;
    int32_t standbyDnsIndex;
    int32_t localIpsIndex;
    int32_t remoteIpsIndex;
    int32_t localPortsIndex;
    int32_t remotePortsIndex;
    int32_t domainsIndex;
    int32_t userIdIndex;
};

// Intercept the structure of records in the database
struct NetInterceptRecordInfo {
    int32_t rowCount;
    int32_t idIndex;
    int32_t timeIndex;
    int32_t localIpIndex;
    int32_t remoteIpIndex;
    int32_t localPortIndex;
    int32_t remotePortIndex;
    int32_t protocolIndex;
    int32_t appUidIndex;
    int32_t domainIndex;
};

class NetFirewallDbHelper : public NoCopyable {
public:
    static NetFirewallDbHelper &GetInstance();

    /**
     * add NetFirewallRule data record
     *
     * @param rule net firewall rule
     * @return Returns 0 success. Otherwise fail
     */
    int32_t AddFirewallRuleRecord(const NetFirewallRule &rule);

    /**
     * Add interception logs
     *
     * @param userId User id
     * @param records intercept records
     * @return Returns 0 success. Otherwise fail
     */
    int32_t AddInterceptRecord(const int32_t userId, std::vector<sptr<InterceptRecord>> &records);

    /**
     * Query enabled rule set
     *
     * @param rules List of rules obtained from query
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryAllUserEnabledFirewallRules(std::vector<NetFirewallRule> &rules,
        NetFirewallRuleType type = NetFirewallRuleType::RULE_ALL);

    /**
     * Query enabled rule set
     *
     * @param userId User id
     * @param appUid The UID of an application or service
     * @param rules List of rules obtained from query
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryEnabledFirewallRules(int32_t userId, int32_t appUid, std::vector<NetFirewallRule> &rules);

    /**
     * Query all rules
     *
     * @param rules List of rules obtained from query
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryAllFirewallRuleRecord(std::vector<NetFirewallRule> &rules);

    /**
     * Query firewall rule
     *
     * @param ruleId Rule id
     * @param userId User id
     * @param rules List of rules obtained from query
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallRuleRecord(int32_t ruleId, int32_t userId, std::vector<NetFirewallRule> &rules);

    /**
     * Paging query firewall rules
     *
     * @param userId User id
     * @param requestParam Pagination query input
     * @param rules List of rules obtained from query
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallRule(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info);

    /**
     * Paging query interception records
     *
     * @param userId User id
     * @param requestParam Pagination query input
     * @param rules List of record obtained from query
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryInterceptRecord(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info);

    /**
     * Query the number of firewall rules for a specified user
     *
     * @param userId User id
     * @param rowCount Number of queries found
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallRuleByUserIdCount(const int32_t userId, int64_t &rowCount);

    /**
     * Query the number of all firewall rules
     *
     * @param rowCount Number of queries found
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallRuleAllCount(int64_t &rowCount);

    /**
     * Query the number of all domain rules
     *
     * @return Number of queries found
     */
    int32_t QueryFirewallRuleAllDomainCount();

    /**
     * Query the number of ambiguous domain names
     *
     * @return Number of queries found
     */
    int32_t QueryFirewallRuleAllFuzzyDomainCount();

    /**
     * Query the number of domain rules by userId
     *
     * @param userId User id
     * @return Number of queries found
     */
    int32_t QueryFirewallRuleDomainByUserIdCount(int32_t userId);

    /**
     * Update firewall rule
     *
     * @param rule firewall ruele
     * @return Returns 0 success. Otherwise fail
     */
    int32_t UpdateFirewallRuleRecord(const NetFirewallRule &rule);

    /**
     * Delete firewall rule
     *
     * @param userId User id
     * @param ruleId Rule id
     * @return Returns 0 success. Otherwise fail
     */
    int32_t DeleteFirewallRuleRecord(int32_t userId, int32_t ruleId);

    /**
     * Delete firewall rule by user id
     *
     * @param userId User id
     * @return Returns 0 success. Otherwise fail
     */
    int32_t DeleteFirewallRuleRecordByUserId(int32_t userId);

    /**
     * Delete firewall rule by app uid
     *
     * @param appUid The UID of an application or service
     * @return Returns 0 success. Otherwise fail
     */
    int32_t DeleteFirewallRuleRecordByAppId(int32_t appUid);

    /**
     * Delete intercept record by user id
     *
     * @param userId User id
     * @return Returns 0 success. Otherwise fail
     */
    int32_t DeleteInterceptRecord(int32_t userId);

    /**
     * Does the specified firewall rule exist
     *
     * @param oldRule Current existing rules
     * @return If there is a return to true, otherwise it will be false
     */
    bool IsFirewallRuleExist(int32_t ruleId, NetFirewallRule &oldRule);

    /**
     * Does the specified dns rule exist
     *
     * @param oldRule Current existing rules
     * @return If there is a return to true, otherwise it will be false
     */
    bool IsDnsRuleExist(const sptr<NetFirewallRule> &rule);

    /**
     * Query the number of query databases
     *
     * @param outValue Number of queries found
     * @param predicates Matching criteria
     * @return Returns 0 success. Otherwise fail
     */
    int32_t Count(int64_t &outValue, const OHOS::NativeRdb::AbsRdbPredicates &predicates);

    int32_t QuerySql(const std::string &sql);

private:
    NetFirewallDbHelper();
	~NetFirewallDbHelper();

    // Fill in firewall rule data
    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule);

    // Check if data needs to be updated
    int32_t CheckIfNeedUpdateEx(const std::string &tableName, bool &isUpdate, int32_t ruleId, NetFirewallRule &oldRule);

    int32_t QueryFirewallRuleRecord(const NativeRdb::RdbPredicates &rdbPredicates,
        const std::vector<std::string> &columns, std::vector<NetFirewallRule> &rules);

    int32_t DeleteAndNoOtherOperation(const std::string &whereClause, const std::vector<std::string> &whereArgs);

    template <typename T>
    int32_t QueryAndGetResult(const NativeRdb::RdbPredicates &rdbPredicates, const std::vector<std::string> &columns,
        std::vector<T> &rules);

    void GetParamRuleInfoFormResultSet(std::string &columnName, int32_t index, NetFirewallRuleInfo &table);

    int32_t GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        struct NetFirewallRuleInfo &table);

    int32_t GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        NetInterceptRecordInfo &table);

    // Convert query result ResultSet
    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<NetFirewallRule> &rules);

    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<InterceptRecord> &rules);

    int32_t AddFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule);

    void GetRuleDataFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        const NetFirewallRuleInfo &table, NetFirewallRule &info);
    void GetRuleListParamFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        const NetFirewallRuleInfo &table, NetFirewallRule &info);
    static bool DomainListToBlob(const std::vector<NetFirewallDomainParam> &vec, std::vector<uint8_t> &blob,
        uint32_t &fuzzyNum);
    static bool BlobToDomainList(const std::vector<uint8_t> &blob, std::vector<NetFirewallDomainParam> &vec);
    template <typename T> static bool ListToBlob(const std::vector<T> &vec, std::vector<uint8_t> &blob);
    template <typename T> static bool BlobToList(const std::vector<uint8_t> &blob, std::vector<T> &vec);

private:
    static std::shared_ptr<NetFirewallDbHelper> instance_;
    std::mutex databaseMutex_;
    std::shared_ptr<NetFirewallDataBase> firewallDatabase_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // FIREWALL_DB_HELPER_H