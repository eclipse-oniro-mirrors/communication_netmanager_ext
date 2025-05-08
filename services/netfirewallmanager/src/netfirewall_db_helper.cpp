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

#include <securec.h>
#include <string>

#include "hilog/log.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_db_helper.h"

using namespace OHOS::NativeRdb;
namespace {
const std::string DATABASE_ID = "id";
const std::string RULE_ID = "ruleId";
const std::string DOMAIN_NUM = "domainNum";
const std::string FUZZY_NUM = "fuzzyDomainNum";
const std::string SQL_SUM = "SELECT SUM(";
const std::string SQL_FROM = ") FROM ";
}

namespace OHOS {
namespace NetManagerStandard {
NetFirewallDbHelper::NetFirewallDbHelper()
{
    firewallDatabase_ = NetFirewallDataBase::GetInstance();
}

NetFirewallDbHelper::~NetFirewallDbHelper()
{
    firewallDatabase_ = nullptr;
}

NetFirewallDbHelper &NetFirewallDbHelper::GetInstance()
{
    static NetFirewallDbHelper instance;
    return instance;
}

bool NetFirewallDbHelper::DomainListToBlob(const std::vector<NetFirewallDomainParam> &vec, std::vector<uint8_t> &blob,
    uint32_t &fuzzyNum)
{
    blob.clear();
    for (const auto &param : vec) {
        if (param.isWildcard) {
            fuzzyNum++;
        }
        // 1 put isWildcard
        blob.emplace_back(param.isWildcard ? 1 : 0);
        // 2 for those with a string type, calculate the string size
        uint16_t size = (uint16_t)(param.domain.length());
        uint8_t *sizePtr = (uint8_t *)&size;
        blob.emplace_back(sizePtr[0]);
        blob.emplace_back(sizePtr[1]);
        // 3 store string
        std::vector<uint8_t> domain(param.domain.begin(), param.domain.end());
        blob.insert(blob.end(), domain.begin(), domain.end());
    }
    return blob.size() > 0;
}

bool NetFirewallDbHelper::BlobToDomainList(const std::vector<uint8_t> &blob, std::vector<NetFirewallDomainParam> &vec)
{
    vec.clear();
    size_t blobSize = blob.size();
    if (blobSize < 1) {
        return false;
    }

    size_t i = 0;
    size_t lenSize = sizeof(uint16_t);
    while (i < blobSize) {
        NetFirewallDomainParam param;
        // 1 get isWildcard
        param.isWildcard = blob[i] ? true : false;
        // 2 get size
        i++;
        if (i >= blobSize || (blobSize - i) < lenSize) {
            return true;
        }
        const uint8_t *sizePtr = &blob[i];
        uint16_t size = *((uint16_t *)sizePtr);
        size_t index = i + lenSize;
        if (index >= blobSize || (blobSize - index) < size) {
            return true;
        }
        // 3 get string
        auto it = blob.begin() + index;
        param.domain = std::string(it, it + size);
        vec.emplace_back(param);
        i += size + lenSize;
    }

    return vec.size() > 0;
}

template <typename T> void NetFirewallDbHelper::ListToBlob(const std::vector<T> &vec, std::vector<uint8_t> &blob)
{
    blob.clear();
    size_t size = sizeof(T);
    for (const auto &param : vec) {
        const uint8_t *data = reinterpret_cast<const uint8_t *>(&param);
        std::vector<uint8_t> item(data, data + size);
        // 1 store each object
        blob.insert(blob.end(), item.begin(), item.end());
    }
}

template <typename T> void NetFirewallDbHelper::BlobToList(const std::vector<uint8_t> &blob, std::vector<T> &vec)
{
    vec.clear();
    size_t blobSize = blob.size();
    if (blobSize < 1) {
        return;
    }

    size_t i = 0;
    size_t size = sizeof(T);
    while (i < blobSize) {
        if ((blobSize - i) < size) {
            return;
        }
        T value;
        memset_s(&value, size, 0, size);
        memcpy_s(&value, size, &blob[i], size);
        vec.emplace_back(value);
        i += size;
    }
}

int32_t NetFirewallDbHelper::FillValuesOfFirewallRule(ValuesBucket &values, const NetFirewallRule &rule)
{
    values.Clear();

    values.PutInt(NET_FIREWALL_USER_ID, rule.userId);
    values.PutString(NET_FIREWALL_RULE_NAME, rule.ruleName);
    values.PutString(NET_FIREWALL_RULE_DESC, rule.ruleDescription);
    values.PutInt(NET_FIREWALL_RULE_DIR, static_cast<int32_t>(rule.ruleDirection));
    values.PutInt(NET_FIREWALL_RULE_ACTION, static_cast<int32_t>(rule.ruleAction));
    values.PutInt(NET_FIREWALL_RULE_TYPE, static_cast<int32_t>(rule.ruleType));
    values.PutInt(NET_FIREWALL_IS_ENABLED, rule.isEnabled);
    values.PutInt(NET_FIREWALL_APP_ID, rule.appUid);
    std::vector<uint8_t> blob;
    std::vector<DataBaseIp> dbIPs;
    std::vector<DataBasePort> dbPorts;
    switch (rule.ruleType) {
        case NetFirewallRuleType::RULE_IP: {
            values.PutInt(NET_FIREWALL_PROTOCOL, static_cast<int32_t>(rule.protocol));
            FirewallIpToDbIp(rule.localIps, dbIPs);
            ListToBlob(dbIPs, blob);
            values.PutBlob(NET_FIREWALL_LOCAL_IP, blob);

            FirewallIpToDbIp(rule.remoteIps, dbIPs);
            ListToBlob(dbIPs, blob);
            values.PutBlob(NET_FIREWALL_REMOTE_IP, blob);

            FirewallPortToDbPort(rule.localPorts, dbPorts);
            ListToBlob(dbPorts, blob);
            values.PutBlob(NET_FIREWALL_LOCAL_PORT, blob);

            FirewallPortToDbPort(rule.remotePorts, dbPorts);
            ListToBlob(dbPorts, blob);
            values.PutBlob(NET_FIREWALL_REMOTE_PORT, blob);
            break;
        }
        case NetFirewallRuleType::RULE_DNS: {
            values.PutString(NET_FIREWALL_DNS_PRIMARY, rule.dns.primaryDns);
            values.PutString(NET_FIREWALL_DNS_STANDY, rule.dns.standbyDns);
            break;
        }
        case NetFirewallRuleType::RULE_DOMAIN: {
            values.PutInt(DOMAIN_NUM, rule.domains.size());
            uint32_t fuzzyNum = 0;
            DomainListToBlob(rule.domains, blob, fuzzyNum);
            values.PutInt(FUZZY_NUM, fuzzyNum);
            values.PutBlob(NET_FIREWALL_RULE_DOMAIN, blob);
            break;
        }
        default:
            break;
    }
    return FIREWALL_OK;
}


int32_t NetFirewallDbHelper::AddFirewallRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule)
{
    FillValuesOfFirewallRule(values, rule);
    return firewallDatabase_->Insert(values, FIREWALL_TABLE_NAME);
}

int32_t NetFirewallDbHelper::AddFirewallRuleRecord(const NetFirewallRule &rule)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    ValuesBucket values;
    int32_t ret = AddFirewallRule(values, rule);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("AddFirewallRule Insert error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
    }
    return ret;
}

int32_t NetFirewallDbHelper::CheckIfNeedUpdateEx(const std::string &tableName, bool &isUpdate, int32_t ruleId,
    NetFirewallRule &oldRule)
{
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(tableName);
    rdbPredicates.BeginWrap()->EqualTo(RULE_ID, std::to_string(ruleId))->EndWrap();
    auto resultSet = firewallDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        NETMGR_EXT_LOG_E("Query error");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t rowCount = 0;
    if (resultSet->GetRowCount(rowCount) != E_OK) {
        NETMGR_EXT_LOG_E("GetRowCount error");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    std::vector<NetFirewallRule> rules;
    GetResultRightRecordEx(resultSet, rules);
    isUpdate = rowCount > 0 && !rules.empty();
    if (!rules.empty()) {
        oldRule.ruleId = rules[0].ruleId;
        oldRule.userId = rules[0].userId;
        oldRule.ruleType = rules[0].ruleType;
        oldRule.isEnabled = rules[0].isEnabled;
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::UpdateFirewallRuleRecord(const NetFirewallRule &rule)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);

    ValuesBucket values;
    FillValuesOfFirewallRule(values, rule);
    int32_t changedRows = 0;
    int32_t ret = firewallDatabase_->Update(FIREWALL_TABLE_NAME, changedRows, values, "ruleId = ?",
        std::vector<std::string> { std::to_string(rule.ruleId) });
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Update error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
    }
    return ret;
}

void NetFirewallDbHelper::GetParamRuleInfoFormResultSet(std::string &columnName, int32_t index,
    NetFirewallRuleInfo &table)
{
    if (columnName == NET_FIREWALL_PROTOCOL) {
        table.protocolIndex = index;
        return;
    }
    if (columnName == NET_FIREWALL_LOCAL_IP) {
        table.localIpsIndex = index;
        return;
    }
    if (columnName == NET_FIREWALL_REMOTE_IP) {
        table.remoteIpsIndex = index;
        return;
    }
    if (columnName == NET_FIREWALL_LOCAL_PORT) {
        table.localPortsIndex = index;
        return;
    }
    if (columnName == NET_FIREWALL_REMOTE_PORT) {
        table.remotePortsIndex = index;
        return;
    }
    if (columnName == NET_FIREWALL_RULE_DOMAIN) {
        table.domainsIndex = index;
        return;
    }
    if (columnName == NET_FIREWALL_DNS_PRIMARY) {
        table.primaryDnsIndex = index;
        return;
    }
    if (columnName == NET_FIREWALL_DNS_STANDY) {
        table.standbyDnsIndex = index;
    }
}

int32_t NetFirewallDbHelper::GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    NetFirewallRuleInfo &table)
{
    std::vector<std::string> columnNames;
    if (resultSet->GetRowCount(table.rowCount) != E_OK || resultSet->GetAllColumnNames(columnNames) != E_OK) {
        NETMGR_EXT_LOG_E("get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &columnName = columnNames.at(i);
        if (columnName == RULE_ID) {
            table.ruleIdIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_USER_ID) {
            table.userIdIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RULE_NAME) {
            table.ruleNameIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RULE_DESC) {
            table.ruleDescriptionIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RULE_DIR) {
            table.ruleDirectionIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RULE_ACTION) {
            table.ruleActionIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RULE_TYPE) {
            table.ruleTypeIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_IS_ENABLED) {
            table.isEnabledIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_APP_ID) {
            table.appUidIndex = i;
            continue;
        }
        GetParamRuleInfoFormResultSet(columnName, i, table);
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    NetInterceptRecordInfo &table)
{
    int32_t rowCount = 0;
    std::vector<std::string> columnNames;
    if (resultSet->GetRowCount(rowCount) != E_OK || resultSet->GetAllColumnNames(columnNames) != E_OK) {
        NETMGR_EXT_LOG_E("get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &columnName = columnNames.at(i);
        if (columnName == NET_FIREWALL_RECORD_TIME) {
            table.timeIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RECORD_LOCAL_IP) {
            table.localIpIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RECORD_REMOTE_IP) {
            table.remoteIpIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RECORD_LOCAL_PORT) {
            table.localPortIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RECORD_REMOTE_PORT) {
            table.remotePortIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RECORD_PROTOCOL) {
            table.protocolIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_RECORD_UID) {
            table.appUidIndex = i;
            continue;
        }
        if (columnName == NET_FIREWALL_DOMAIN) {
            table.domainIndex = i;
        }
    }
    table.rowCount = rowCount;
    return FIREWALL_OK;
}

void NetFirewallDbHelper::GetRuleDataFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    const NetFirewallRuleInfo &table, NetFirewallRule &info)
{
    resultSet->GetInt(table.userIdIndex, info.userId);
    resultSet->GetString(table.ruleNameIndex, info.ruleName);
    resultSet->GetString(table.ruleDescriptionIndex, info.ruleDescription);
    int ruleDirection = 0;
    if (resultSet->GetInt(table.ruleDirectionIndex, ruleDirection) == E_OK) {
        info.ruleDirection = static_cast<NetFirewallRuleDirection>(ruleDirection);
    }
    int ruleAction = 0;
    if (resultSet->GetInt(table.ruleActionIndex, ruleAction) == E_OK) {
        info.ruleAction = static_cast<FirewallRuleAction>(ruleAction);
    }
    int ruleType = 0;
    if (resultSet->GetInt(table.ruleTypeIndex, ruleType) == E_OK) {
        info.ruleType = static_cast<NetFirewallRuleType>(ruleType);
    }
    int isEnabled = 0;
    if (resultSet->GetInt(table.isEnabledIndex, isEnabled) == E_OK) {
        info.isEnabled = static_cast<bool>(isEnabled);
    }
    resultSet->GetInt(table.appUidIndex, info.appUid);
    GetRuleListParamFromResultSet(resultSet, table, info);
}

void NetFirewallDbHelper::GetRuleListParamFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    const NetFirewallRuleInfo &table, NetFirewallRule &info)
{
    std::vector<uint8_t> value;
    std::vector<DataBaseIp> dbIPs;
    std::vector<DataBasePort> dbPorts;
    switch (info.ruleType) {
        case NetFirewallRuleType::RULE_IP: {
            int protocol = 0;
            if (resultSet->GetInt(table.protocolIndex, protocol) == E_OK) {
                info.protocol = static_cast<NetworkProtocol>(protocol);
            }
            resultSet->GetBlob(table.localIpsIndex, value);
            BlobToList(value, dbIPs);
            DbIpToFirewallIp(dbIPs, info.localIps);
            value.clear();
            resultSet->GetBlob(table.remoteIpsIndex, value);
            BlobToList(value, dbIPs);
            DbIpToFirewallIp(dbIPs, info.remoteIps);
            value.clear();
            resultSet->GetBlob(table.localPortsIndex, value);
            BlobToList(value, dbPorts);
            DbPortToFirewallPort(dbPorts, info.localPorts);
            value.clear();
            resultSet->GetBlob(table.remotePortsIndex, value);
            BlobToList(value, dbPorts);
            DbPortToFirewallPort(dbPorts, info.remotePorts);
            break;
        }
        case NetFirewallRuleType::RULE_DNS: {
            resultSet->GetString(table.primaryDnsIndex, info.dns.primaryDns);
            resultSet->GetString(table.standbyDnsIndex, info.dns.standbyDns);
            break;
        }

        case NetFirewallRuleType::RULE_DOMAIN: {
            resultSet->GetBlob(table.domainsIndex, value);
            BlobToDomainList(value, info.domains);
            break;
        }
        default:
            break;
    }
}

int32_t NetFirewallDbHelper::GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<NetFirewallRule> &rules)
{
    NetFirewallRuleInfo table;
    int32_t ret = GetResultSetTableInfo(resultSet, table);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("GetResultSetTableInfo failed");
        return ret;
    }

    bool endFlag = false;
    NetFirewallRule info;
    for (int32_t i = 0; (i < table.rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GoToRow %{public}d", i);
            break;
        }
        resultSet->GetInt(table.ruleIdIndex, info.ruleId);
        if (info.ruleId > 0) {
            GetRuleDataFromResultSet(resultSet, table, info);
            rules.emplace_back(std::move(info));
        }

        resultSet->IsEnded(endFlag);
    }
    resultSet->Close();
    return rules.size();
}

int32_t NetFirewallDbHelper::GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<InterceptRecord> &rules)
{
    NetInterceptRecordInfo table;
    int32_t ret = GetResultSetTableInfo(resultSet, table);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("GetResultSetTableInfo failed");
        return ret;
    }

    bool endFlag = false;
    int32_t localPort = 0;
    int32_t remotePort = 0;
    int32_t protocol = 0;
    InterceptRecord info;
    for (int32_t i = 0; (i < table.rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GetResultRightRecordEx GoToRow %{public}d", i);
            break;
        }
        resultSet->GetInt(table.timeIndex, info.time);
        resultSet->GetString(table.localIpIndex, info.localIp);
        resultSet->GetString(table.remoteIpIndex, info.remoteIp);
        if (resultSet->GetInt(table.localPortIndex, localPort) == E_OK) {
            info.localPort = static_cast<uint16_t>(localPort);
        }
        if (resultSet->GetInt(table.remotePortIndex, remotePort) == E_OK) {
            info.remotePort = static_cast<uint16_t>(remotePort);
        }
        if (resultSet->GetInt(table.protocolIndex, protocol) == E_OK) {
            info.protocol = static_cast<uint16_t>(protocol);
        }
        resultSet->GetInt(table.appUidIndex, info.appUid);
        resultSet->GetString(table.domainIndex, info.domain);
        if (info.time > 0) {
            rules.emplace_back(std::move(info));
        }
        resultSet->IsEnded(endFlag);
    }
    int32_t index = 0;
    resultSet->GetRowIndex(index);
    resultSet->IsEnded(endFlag);
    NETMGR_EXT_LOG_I("row=%{public}d pos=%{public}d ret=%{public}zu end=%{public}s", table.rowCount, index,
        rules.size(), (endFlag ? "yes" : "no"));

    resultSet->Close();
    return rules.size();
}

template <typename T>
int32_t NetFirewallDbHelper::QueryAndGetResult(const NativeRdb::RdbPredicates &rdbPredicates,
    const std::vector<std::string> &columns, std::vector<T> &rules)
{
    auto resultSet = firewallDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        NETMGR_EXT_LOG_E("Query error");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    return GetResultRightRecordEx(resultSet, rules);
}

int32_t NetFirewallDbHelper::QueryAllFirewallRuleRecord(std::vector<NetFirewallRule> &rules)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    NETMGR_EXT_LOG_I("Query detail: all user");
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    return QueryFirewallRuleRecord(rdbPredicates, columns, rules);
}

int32_t NetFirewallDbHelper::QueryAllUserEnabledFirewallRules(std::vector<NetFirewallRule> &rules,
    NetFirewallRuleType type)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    NETMGR_EXT_LOG_I("Query detail: all user");
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()->EqualTo(NET_FIREWALL_IS_ENABLED, "1");
    if (type != NetFirewallRuleType::RULE_ALL && type != NetFirewallRuleType::RULE_INVALID) {
        rdbPredicates.And()->EqualTo(NET_FIREWALL_RULE_TYPE, std::to_string(static_cast<int32_t>(type)));
    }
    rdbPredicates.EndWrap();
    return QueryFirewallRuleRecord(rdbPredicates, columns, rules);
}

int32_t NetFirewallDbHelper::QueryEnabledFirewallRules(int32_t userId, int32_t appUid,
    std::vector<NetFirewallRule> &rules)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    NETMGR_EXT_LOG_I("QueryEnabledFirewallRules : userId=%{public}d ", userId);
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()
        ->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId))
        ->And()
        ->EqualTo(NET_FIREWALL_IS_ENABLED, "1")
        ->And()
        ->EqualTo(NET_FIREWALL_APP_ID, appUid)
        ->EndWrap();
    return QueryFirewallRuleRecord(rdbPredicates, columns, rules);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleRecord(int32_t ruleId, int32_t userId,
    std::vector<NetFirewallRule> &rules)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    NETMGR_EXT_LOG_I("Query detail: ruleId=%{public}d userId=%{public}d", ruleId, userId);
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()
        ->EqualTo(RULE_ID, std::to_string(ruleId))
        ->And()
        ->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId))
        ->EndWrap();

    return QueryFirewallRuleRecord(rdbPredicates, columns, rules);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleRecord(const NativeRdb::RdbPredicates &rdbPredicates,
    const std::vector<std::string> &columns, std::vector<NetFirewallRule> &rules)
{
    int32_t ret = QueryAndGetResult(rdbPredicates, columns, rules);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("QueryFirewallRuleRecord error.");
        return ret;
    }
    size_t size = rules.size();
    if (size == 0) {
        NETMGR_EXT_LOG_I("QueryFirewallRuleRecord rule empty");
        return FIREWALL_OK;
    }
    NETMGR_EXT_LOG_I("QueryFirewallRuleRecord rule size: %{public}zu", size);
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::DeleteAndNoOtherOperation(const std::string &whereClause,
    const std::vector<std::string> &whereArgs)
{
    int32_t changedRows = 0;
    int32_t ret = firewallDatabase_->Delete(FIREWALL_TABLE_NAME, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK) {
        (void)firewallDatabase_->RollBack();
        return FIREWALL_FAILURE;
    }
    return ret;
}

int32_t NetFirewallDbHelper::DeleteFirewallRuleRecord(int32_t userId, int32_t ruleId)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::string whereClause = { "userId = ? AND ruleId = ?" };
    std::vector<std::string> whereArgs = { std::to_string(userId), std::to_string(ruleId) };
    int32_t ret = DeleteAndNoOtherOperation(whereClause, whereArgs);
    if (ret != FIREWALL_OK) {
        NETMGR_EXT_LOG_E("failed: detale(ruleId): %{public}d", ret);
    }
    return ret;
}

int32_t NetFirewallDbHelper::DeleteFirewallRuleRecordByUserId(int32_t userId)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::string whereClause = { "userId = ?" };
    std::vector<std::string> whereArgs = { std::to_string(userId) };
    int32_t ret = DeleteAndNoOtherOperation(whereClause, whereArgs);
    if (ret != FIREWALL_OK) {
        NETMGR_EXT_LOG_E("failed: detale(ruleId): %{public}d", ret);
    }
    return ret;
}

int32_t NetFirewallDbHelper::DeleteFirewallRuleRecordByAppId(int32_t appUid)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::string whereClause = { "appUid = ?" };
    std::vector<std::string> whereArgs = { std::to_string(appUid) };
    int32_t ret = DeleteAndNoOtherOperation(whereClause, whereArgs);
    if (ret != FIREWALL_OK) {
        NETMGR_EXT_LOG_E("failed: detale(ruleId): %{public}d", ret);
    }
    return ret;
}

bool NetFirewallDbHelper::IsFirewallRuleExist(int32_t ruleId, NetFirewallRule &oldRule)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    bool isExist = false;
    int32_t ret = CheckIfNeedUpdateEx(FIREWALL_TABLE_NAME, isExist, ruleId, oldRule);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("check if need update error: %{public}d", ret);
    }
    return isExist;
}

int32_t NetFirewallDbHelper::QueryFirewallRuleByUserIdCount(int32_t userId, int64_t &rowCount)
{
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId))->EndWrap();

    return Count(rowCount, rdbPredicates);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleAllCount(int64_t &rowCount)
{
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    return Count(rowCount, rdbPredicates);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleAllDomainCount()
{
    return QuerySql(SQL_SUM + DOMAIN_NUM + SQL_FROM + FIREWALL_TABLE_NAME);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleAllFuzzyDomainCount()
{
    return QuerySql(SQL_SUM + FUZZY_NUM + SQL_FROM + FIREWALL_TABLE_NAME);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleDomainByUserIdCount(int32_t userId)
{
    return QuerySql(SQL_SUM + DOMAIN_NUM + SQL_FROM + FIREWALL_TABLE_NAME + " WHERE (" + NET_FIREWALL_USER_ID + " = " +
        std::to_string(userId) + ")");
}

int32_t NetFirewallDbHelper::QueryFirewallRule(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int64_t rowCount = 0;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId))->EndWrap();
    firewallDatabase_->Count(rowCount, rdbPredicates);
    if (rowCount == 0) {
        NETMGR_EXT_LOG_I("QueryFirewallRule: no rule found");
        return FIREWALL_OK;
    }
    info->totalPage = rowCount / requestParam->pageSize;
    int32_t remainder = rowCount % requestParam->pageSize;
    if (remainder > 0) {
        info->totalPage += 1;
    }
    NETMGR_EXT_LOG_I("QueryFirewallRule: userId=%{public}d page=%{public}d pageSize=%{public}d total=%{public}d",
        userId, requestParam->page, requestParam->pageSize, info->totalPage);
    if (info->totalPage < requestParam->page) {
        return FIREWALL_FAILURE;
    }
    std::vector<std::string> columns;
    rdbPredicates.Clear();
    rdbPredicates.BeginWrap()->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId));
    if (requestParam->orderType == NetFirewallOrderType::ORDER_ASC) {
        rdbPredicates.OrderByAsc(NET_FIREWALL_RULE_NAME);
    } else {
        rdbPredicates.OrderByDesc(NET_FIREWALL_RULE_NAME);
    }
    rdbPredicates.Limit((requestParam->page - 1) * requestParam->pageSize, requestParam->pageSize)->EndWrap();
    return QueryFirewallRuleRecord(rdbPredicates, columns, info->data);
}

int32_t NetFirewallDbHelper::Count(int64_t &outValue, const OHOS::NativeRdb::AbsRdbPredicates &predicates)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int32_t ret = firewallDatabase_->Count(outValue, predicates);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Count error");
        return -1;
    }
    return ret;
}

int32_t NetFirewallDbHelper::QuerySql(const std::string &sql)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::vector<std::string> selectionArgs;
    auto resultSet = firewallDatabase_->QuerySql(sql, selectionArgs);
    if (resultSet == nullptr) {
        NETMGR_EXT_LOG_E("QuerySql error");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t rowCount = 0;
    if (resultSet->GetRowCount(rowCount) != E_OK || resultSet->GoToRow(0) != E_OK) {
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t value = 0;
    resultSet->GetInt(0, value);
    return value;
}

bool NetFirewallDbHelper::IsDnsRuleExist(const sptr<NetFirewallRule> &rule)
{
    if (rule->ruleType != NetFirewallRuleType::RULE_DNS) {
        return false;
    }
    std::lock_guard<std::mutex> guard(databaseMutex_);
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()
        ->EqualTo(NET_FIREWALL_USER_ID, std::to_string(rule->userId))
        ->And()
        ->EqualTo(NET_FIREWALL_RULE_TYPE, std::to_string(static_cast<int32_t>(rule->ruleType)))
        ->And()
        ->EqualTo(NET_FIREWALL_APP_ID, std::to_string(rule->appUid))
        ->And()
        ->BeginWrap()
        ->EqualTo(NET_FIREWALL_DNS_PRIMARY, rule->dns.primaryDns)
        ->Or()
        ->EqualTo(NET_FIREWALL_DNS_STANDY, rule->dns.standbyDns)
        ->EndWrap()
        ->Limit(1)
        ->EndWrap();
    std::vector<std::string> columns;
    auto resultSet = firewallDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        NETMGR_EXT_LOG_E("IsDnsRuleExist Query error");
        return false;
    }
    int32_t rowCount = 0;
    resultSet->GetRowCount(rowCount);
    return rowCount > 0;
}

int32_t NetFirewallDbHelper::AddInterceptRecord(const int32_t userId, std::vector<sptr<InterceptRecord>> &records)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int32_t ret = firewallDatabase_->BeginTransaction();
    // Aging by date, record up to 8 days of data
    std::string whereClause = { "userId = ? AND time < ?" };
    std::vector<std::string> whereArgs = { std::to_string(userId),
        std::to_string(records.back()->time - RECORD_MAX_SAVE_TIME) };
    int32_t changedRows = 0;
    ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);

    int64_t currentRows = 0;
    RdbPredicates rdbPredicates(INTERCEPT_RECORD_TABLE);
    rdbPredicates.BeginWrap()->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId))->EndWrap();
    firewallDatabase_->Count(currentRows, rdbPredicates);
    // Aging by number, record up to 1000 pieces of data
    size_t size = records.size();
    size_t leftRows = static_cast<size_t>(RECORD_MAX_DATA_NUM - currentRows);
    if (leftRows < size) {
        std::string whereClause("id in (select id from ");
        whereClause += INTERCEPT_RECORD_TABLE;
        whereClause += " where userId = ? order by id limit ? )";
        std::vector<std::string> whereArgs = { std::to_string(userId), std::to_string(size - leftRows) };
        ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);
    }
    // New data written to the database
    ValuesBucket values;
    for (size_t i = 0; i < size; i++) {
        values.Clear();
        values.PutInt(NET_FIREWALL_USER_ID, userId);
        values.PutInt(NET_FIREWALL_RECORD_TIME, records[i]->time);
        values.PutString(NET_FIREWALL_RECORD_LOCAL_IP, records[i]->localIp);
        values.PutString(NET_FIREWALL_RECORD_REMOTE_IP, records[i]->remoteIp);
        values.PutInt(NET_FIREWALL_RECORD_LOCAL_PORT, static_cast<int32_t>(records[i]->localPort));
        values.PutInt(NET_FIREWALL_RECORD_REMOTE_PORT, static_cast<int32_t>(records[i]->remotePort));
        values.PutInt(NET_FIREWALL_RECORD_PROTOCOL, static_cast<int32_t>(records[i]->protocol));
        values.PutInt(NET_FIREWALL_RECORD_UID, records[i]->appUid);
        values.PutString(NET_FIREWALL_DOMAIN, records[i]->domain);

        ret = firewallDatabase_->Insert(values, INTERCEPT_RECORD_TABLE);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("AddInterceptRecord error: %{public}d", ret);
            firewallDatabase_->Commit();
            return -1;
        }
    }
    return firewallDatabase_->Commit();
}

int32_t NetFirewallDbHelper::DeleteInterceptRecord(const int32_t userId)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::string whereClause = { "userId = ?" };
    std::vector<std::string> whereArgs = { std::to_string(userId) };
    int32_t changedRows = 0;
    int32_t ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("DeleteInterceptRecord error: %{public}d", ret);
        return -1;
    }
    return ret;
}

int32_t NetFirewallDbHelper::QueryInterceptRecord(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int64_t rowCount = 0;
    RdbPredicates rdbPredicates(INTERCEPT_RECORD_TABLE);
    rdbPredicates.BeginWrap()->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId))->EndWrap();
    firewallDatabase_->Count(rowCount, rdbPredicates);
    info->totalPage = rowCount / requestParam->pageSize;
    int32_t remainder = rowCount % requestParam->pageSize;
    if (remainder > 0) {
        info->totalPage += 1;
    }
    NETMGR_EXT_LOG_I("QueryInterceptRecord: userId=%{public}d page=%{public}d pageSize=%{public}d total=%{public}d",
        userId, requestParam->page, requestParam->pageSize, info->totalPage);
    if (info->totalPage < requestParam->page) {
        return FIREWALL_FAILURE;
    }
    info->page = requestParam->page;
    std::vector<std::string> columns;
    rdbPredicates.Clear();
    rdbPredicates.BeginWrap()->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId));
    if (requestParam->orderType == NetFirewallOrderType::ORDER_ASC) {
        rdbPredicates.OrderByAsc(NET_FIREWALL_RECORD_TIME);
    } else {
        rdbPredicates.OrderByDesc(NET_FIREWALL_RECORD_TIME);
    }
    rdbPredicates.Limit((requestParam->page - 1) * requestParam->pageSize, requestParam->pageSize)->EndWrap();
    return QueryAndGetResult(rdbPredicates, columns, info->data);
}

void NetFirewallDbHelper::FirewallIpToDbIp(const std::vector<NetFirewallIpParam> &ips, std::vector<DataBaseIp> &dbips)
{
    dbips.clear();
    DataBaseIp dbip;
    for (const NetFirewallIpParam &param : ips) {
        dbip.family = param.family;
        dbip.mask = param.mask;
        dbip.type = param.type;
        if (dbip.family == FAMILY_IPV4) {
            memcpy_s(&dbip.ipv4.startIp, sizeof(uint32_t), &param.ipv4.startIp, sizeof(uint32_t));
            memcpy_s(&dbip.ipv4.endIp, sizeof(uint32_t), &param.ipv4.endIp, sizeof(uint32_t));
        } else {
            memcpy_s(&dbip.ipv6.startIp, sizeof(in6_addr), &param.ipv6.startIp, sizeof(in6_addr));
            memcpy_s(&dbip.ipv6.endIp, sizeof(in6_addr), &param.ipv6.endIp, sizeof(in6_addr));
        }
        dbips.emplace_back(std::move(dbip));
    }
}
void NetFirewallDbHelper::DbIpToFirewallIp(const std::vector<DataBaseIp> &dbips, std::vector<NetFirewallIpParam> &ips)
{
    ips.clear();
    NetFirewallIpParam dbip;
    for (const DataBaseIp &param : dbips) {
        dbip.family = param.family;
        dbip.mask = param.mask;
        dbip.type = param.type;
        if (dbip.family == FAMILY_IPV4) {
            memcpy_s(&dbip.ipv4.startIp, sizeof(uint32_t), &param.ipv4.startIp, sizeof(uint32_t));
            memcpy_s(&dbip.ipv4.endIp, sizeof(uint32_t), &param.ipv4.endIp, sizeof(uint32_t));
        } else {
            memcpy_s(&dbip.ipv6.startIp, sizeof(in6_addr), &param.ipv6.startIp, sizeof(in6_addr));
            memcpy_s(&dbip.ipv6.endIp, sizeof(in6_addr), &param.ipv6.endIp, sizeof(in6_addr));
        }
        ips.emplace_back(std::move(dbip));
    }
}
void NetFirewallDbHelper::FirewallPortToDbPort(const std::vector<NetFirewallPortParam> &ports,
    std::vector<DataBasePort> &dbports)
{
    dbports.clear();
    DataBasePort dbport;
    for (const NetFirewallPortParam &param : ports) {
        dbport.startPort = param.startPort;
        dbport.endPort = param.endPort;
        dbports.emplace_back(std::move(dbport));
    }
}

void NetFirewallDbHelper::DbPortToFirewallPort(const std::vector<DataBasePort> &dbports,
    std::vector<NetFirewallPortParam> &ports)
{
    ports.clear();
    NetFirewallPortParam dbport;
    for (const DataBasePort &param : dbports) {
        dbport.startPort = param.startPort;
        dbport.endPort = param.endPort;
        ports.emplace_back(std::move(dbport));
    }
}
} // namespace NetManagerStandard
} // namespace OHOS