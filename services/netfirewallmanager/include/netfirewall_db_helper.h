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

#include <string>

namespace OHOS {
namespace NetManagerStandard {
// 防火墙规则，存储数据库使用
struct NetFirewallRuleData {
    int32_t ruleId;                         // 规则ID
    std::string ruleName;                   // 规则名称，必填
    std::string ruleDescription;            // 规则描述，可选
    NetFirewallRuleDirection ruleDirection; // 规则方向，入站或者出站
    FirewallRuleAction ruleAction;          // 行为规则
    NetFirewallRuleType ruleType;           // 规则类型，必填
    bool isEnabled;                         // 是否启用
    int32_t appUid;                         // 应用或服务ID
    NetworkProtocol protocol;               // 协议，TCP：6，UDP：17，...。
    NetFirewallDnsParam dns;                // DNS列表
    int32_t userId;                         // 用户ID，必填
};

struct NetFirewallRuleInfo {
    int32_t rowCount;
    int32_t columnCount;
    int32_t ruleIdIndex;
    int32_t ruleNameIndex;        // 规则名称，必填
    int32_t ruleDescriptionIndex; // 规则描述，可选
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

enum class LocationType {
    SRC_LOCATION = 0,
    DST_LOCATION
};

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

    /* add NetFirewallRuleData record */
    int32_t AddFirewallRuleRecord(const NetFirewallRule &rule);
    int32_t AddInterceptRecord(const int32_t userId, std::vector<sptr<InterceptRecord>> &records);

    /* add or update NetFirewallRuleData record */
    int32_t QueryEnabledFirewallRules(int32_t userId, std::vector<NetFirewallRule> &rules);
    int32_t QueryEnabledFirewallRules(int32_t userId, int32_t appUid, std::vector<NetFirewallRule> &rules);
    int32_t QueryEnabledDomainOrDnsRules(int32_t userId, std::vector<NetFirewallRule> &rules);
    int32_t QueryAllFirewallRuleRecord(std::vector<NetFirewallRule> &rules);
    int32_t QueryFirewallRuleRecord(int32_t ruleId, int32_t userId, std::vector<NetFirewallRule> &rules);
    int32_t QueryFirewallRule(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<FirewallRulePage> &info);
    int32_t QueryFirewallIpRuleRecord(int32_t userId, std::vector<NetFirewallIpRuleData> &srcIpRules,
        std::vector<NetFirewallIpRuleData> &dstIpRules);
    int32_t QueryFirewallPortRuleRecord(int32_t userId, std::vector<NetFirewallPortRuleData> &srcPortRules,
        std::vector<NetFirewallPortRuleData> &dstProtRules);
    int32_t QueryFirewallDomainRuleRecord(int32_t userId, std::vector<NetFirewallDomainRuleData> &domainRules);
    int32_t QueryFirewallAllDomainRuleRecord(std::vector<NetFirewallDomainParam> &domainRules);
    int32_t QueryInterceptRecord(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info);

    /* update NetFirewallRuleData record */
    int32_t UpdateFirewallRuleRecord(const NetFirewallRule &rule);

    /* delete NetFirewallRuleData record */
    int32_t DeleteFirewallRuleRecord(int32_t userId, int32_t ruleId);
    int32_t DeleteFirewallRuleRecordByUserId(int32_t userId);
    int32_t DeleteFirewallRuleRecordByAppId(int32_t appUid);
    int32_t DeleteInterceptRecord(int32_t userId);

    int32_t QueryFirewallRuleByUserIdCount(const int32_t userId, int64_t &rowCount);

    int32_t QueryFirewallRuleAllCount(int64_t &rowCount);

    int32_t QueryFirewallRuleAllDomainCount(int64_t &rowCount);

    bool IsFirewallRuleExits(int32_t ruleId, NetFirewallRule &oldRule);
    bool IsDnsRuleExist(const sptr<NetFirewallRule> &rule);

    int32_t Count(int64_t &outValue, const OHOS::NativeRdb::AbsRdbPredicates &predicates);

private:
    NetFirewallDbHelper();

    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallRuleData &rule);

    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallIpRuleData &rule);
    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallPortRuleData &rule);
    int32_t FillValuesOfFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallDomainRuleData &rule);

    int32_t CheckIfNeedUpdateEx(const std::string &tableName, bool &isUpdate, int32_t ruleId, NetFirewallRule &oldRule);

    int32_t GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<NetFirewallRuleData> &rules);
    int32_t QueryAndGetResult(const NativeRdb::RdbPredicates &rdbPredicates, const std::vector<std::string> &columns,
        std::vector<NetFirewallRuleData> &rules);

    int32_t QueryFirewallRuleRecord(const NativeRdb::RdbPredicates &rdbPredicates,
        const std::vector<std::string> &columns, std::vector<NetFirewallRule> &rules, bool isQuerySub = true);

    int32_t DeleteAndNoOtherOperation(const std::string &whereClause, const std::vector<std::string> &whereArgs);

    int32_t DeleteFirewallRuleRecordByTable(const std::string &whereClause, const std::vector<std::string> &whereArgs,
        const std::string &tableName);
    template <typename T>
    int32_t QueryAndGetResult(const NativeRdb::RdbPredicates &rdbPredicates, const std::vector<std::string> &columns,
        std::vector<T> &rules);

    int32_t GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        struct NetFirewallRuleInfo &table);
    int32_t GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        NetInterceptRecordInfo &table);
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
    uint64_t callStartTime_ = 0;
    inline uint64_t GetCurrentMilliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    }
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // FIREWALL_DB_HELPER_H