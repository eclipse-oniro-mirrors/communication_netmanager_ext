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
// Firewall rules for storing database usage
struct NetFirewallRuleData {
    int32_t ruleId;
    std::string ruleName;
    std::string ruleDescription;
    NetFirewallRuleDirection ruleDirection;
    FirewallRuleAction ruleAction;
    NetFirewallRuleType ruleType;
    bool isEnabled;
    int32_t appUid;
    NetworkProtocol protocol;
    NetFirewallDnsParam dns;
    int32_t userId;
};

// The data index of NetFirewallRuleData
struct NetFirewallRuleInfo {
    int32_t rowCount;
    int32_t columnCount;
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
    int32_t userIdIndex;
};

// Direction: source or destination
enum class LocationType {
    SRC_LOCATION = 0,
    DST_LOCATION
};

// IP parameter data
struct NetFirewallIpRuleData {
    int32_t ruleId;
    int32_t userId;
    int32_t appUid;
    LocationType locationType;
    int32_t family;
    int32_t type;
    std::string address;
    int32_t mask;
    std::string startIp;
    std::string endIp;
};

struct NetFirewallIpRuleInfo {
    int32_t rowCount;
    int32_t columnCount;
    int32_t idIndex;
    int32_t ruleIdIndex;
    int32_t userIdIndex;
    int32_t appUidIndex;
    int32_t locationTypeIndex;
    int32_t familyIndex;
    int32_t typeIndex;
    int32_t addressIndex;
    int32_t maskIndex;
    int32_t startIpIndex;
    int32_t endIpIndex;
};

// port parameter data
struct NetFirewallPortRuleData {
    int32_t ruleId;
    int32_t userId;
    int32_t appUid;
    LocationType locationType;
    int32_t startPort;
    int32_t endPort;
};

struct NetFirewallPortRuleInfo {
    int32_t rowCount;
    int32_t columnCount;
    int32_t idIndex;
    int32_t ruleIdIndex;
    int32_t userIdIndex;
    int32_t appUidIndex;
    int32_t locationTypeIndex;
    int32_t startPortIndex;
    int32_t endPortIndex;
};

// Domain parameter data
struct NetFirewallDomainRuleData {
    int32_t ruleId;
    int32_t userId;
    int32_t appUid;
    bool isWildcard;
    std::string domain;
};

struct NetFirewallDomainRuleInfo {
    int32_t rowCount;
    int32_t columnCount;
    int32_t idIndex;
    int32_t ruleIdIndex;
    int32_t userIdIndex;
    int32_t appUidIndex;
    int32_t isWildcardIndex;
    int32_t domainIndex;
};

// Intercept the structure of records in the database
struct NetInterceptRecordInfo {
    int32_t rowCount;
    int32_t columnCount;
    int32_t idIndex;
    int32_t timeIndex;
    int32_t sourceIpIndex;
    int32_t destIpIndex;
    int32_t sourcePortIndex;
    int32_t destPortIndex;
    int32_t protocolIndex;
    int32_t appUidIndex;
    int32_t domainIndex;
};

class NetFirewallDbHelper : public NoCopyable {
public:
    static std::shared_ptr<NetFirewallDbHelper> GetInstance();

    /**
     * add NetFirewallRuleData record
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
     * @param userId User id
     * @param rules List of rules obtained from query
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryEnabledFirewallRules(int32_t userId, std::vector<NetFirewallRule> &rules,
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
     * Query IP rules
     *
     * @param userId User id
     * @param srcIpRules The source IP list of the firewall
     * @param dstIpRules The destination IP list of the firewall
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallIpRuleRecord(int32_t userId, std::vector<NetFirewallIpRuleData> &srcIpRules,
        std::vector<NetFirewallIpRuleData> &dstIpRules);

    /**
     * Query port rules
     *
     * @param userId User id
     * @param srcPortRules The source port list of the firewall
     * @param dstProtRules The destination port list of the firewall
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallPortRuleRecord(int32_t userId, std::vector<NetFirewallPortRuleData> &srcPortRules,
        std::vector<NetFirewallPortRuleData> &dstProtRules);

    /**
     * Query domain rules
     *
     * @param userId User id
     * @param domainRules Domain name list for firewall
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallDomainRuleRecord(int32_t userId, std::vector<NetFirewallDomainRuleData> &domainRules);

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
     * @param rowCount Number of queries found
     * @return Returns 0 success. Otherwise fail
     */
    int32_t QueryFirewallRuleAllDomainCount(int64_t &rowCount);

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
    bool IsFirewallRuleExits(int32_t ruleId, NetFirewallRule &oldRule);

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

private:
    NetFirewallDbHelper();

    // Fill in firewall rule data
    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallRuleData &rule);

    // Fill in ip rule data
    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallIpRuleData &rule);

    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallPortRuleData &rule);

    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallDomainRuleData &rule);

    // Check if data needs to be updated
    int32_t CheckIfNeedUpdateEx(const std::string &tableName, bool &isUpdate, int32_t ruleId, NetFirewallRule &oldRule);

    int32_t QueryFirewallRuleRecord(const NativeRdb::RdbPredicates &rdbPredicates,
        const std::vector<std::string> &columns, std::vector<NetFirewallRule> &rules, bool isQuerySub = true);

    int32_t DeleteAndNoOtherOperation(const std::string &whereClause, const std::vector<std::string> &whereArgs);

    template <typename T>
    int32_t QueryAndGetResult(const NativeRdb::RdbPredicates &rdbPredicates, const std::vector<std::string> &columns,
        std::vector<T> &rules);

    int32_t GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        struct NetFirewallRuleInfo &table);

    int32_t GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        NetInterceptRecordInfo &table);

    // Convert query result ResultSet
    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<NetFirewallRuleData> &rules);

    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<NetFirewallDomainRuleData> &rules);

    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<NetFirewallPortRuleData> &rules);

    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<NetFirewallIpRuleData> &rules);

    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<InterceptRecord> &rules);

    int32_t GetResultSetDomainTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        struct NetFirewallDomainRuleInfo &table);

    int32_t GetResultSetPortTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        struct NetFirewallPortRuleInfo &table);

    int32_t GetResultSetIpTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        struct NetFirewallIpRuleInfo &table);

    int32_t AddFirewallBaseRule(NativeRdb::ValuesBucket &values, const NetFirewallRuleData &rule);

    int32_t AddFirewallIpRule(NativeRdb::ValuesBucket &values, const NetFirewallIpRuleData &rule);

    int32_t AddFirewallPortRule(NativeRdb::ValuesBucket &values, const NetFirewallPortRuleData &rule);

    int32_t AddFirewallDomainRule(NativeRdb::ValuesBucket &values, const NetFirewallDomainRuleData &rule);

    int32_t AddFirewallIpRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule, int32_t ruleId,
        LocationType locationType);

    int32_t AddFirewallPortRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule, int32_t ruleId,
        LocationType locationType);

    int32_t AddFirewallDomainRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule, int32_t ruleId);

    void NetFirewallRule2Data(const NetFirewallRule &rule, NetFirewallRuleData &baseRuleData);

    void NetFirewallData2Rule(const NetFirewallRuleData &ruleData, NetFirewallRule &rule);

    int32_t SubTableDelete4UpdateRule(int32_t ruleId);

    int32_t SubTableAdd4UpdateRule(const NetFirewallRule &rule, NativeRdb::ValuesBucket &values);

    int32_t SubTableAddIpParam4UpdateRule(const NetFirewallRule &rule, LocationType locationType,
        NativeRdb::ValuesBucket &values);

    int32_t SubTableAddPortParam4UpdateRule(const NetFirewallRule &rule, LocationType locationType,
        NativeRdb::ValuesBucket &values);

    void GetRuleDataFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        const NetFirewallRuleInfo &table, NetFirewallRuleData &info);

    void GetFirewallIpRuleRecord(int32_t ruleId, const std::vector<NetFirewallIpRuleData> &inIpRules,
        std::vector<NetFirewallIpParam> &outIpRules);

    void GetFirewallRuleIpSub(const std::vector<NetFirewallIpRuleData> localIps,
        const std::vector<NetFirewallIpRuleData> remoteIps, const std::vector<NetFirewallPortRuleData> &localPorts,
        const std::vector<NetFirewallPortRuleData> &remotePorts, NetFirewallRule &rule);

    void GetFirewallPortRuleRecord(int32_t ruleId, const std::vector<NetFirewallPortRuleData> &inPortRules,
        std::vector<NetFirewallPortParam> &outProtRules);

    void GetFirewallDomainRuleRecord(int32_t ruleId, std::vector<NetFirewallDomainRuleData> &inDomainRules,
        std::vector<NetFirewallDomainParam> &outDomainRules);

private:
    static std::shared_ptr<NetFirewallDbHelper> instance_;
    std::mutex databaseMutex_;
    std::shared_ptr<NetFirewallDataBase> firewallDatabase_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // FIREWALL_DB_HELPER_H