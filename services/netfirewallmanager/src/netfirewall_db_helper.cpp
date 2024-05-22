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

#include "netfirewall_db_helper.h"
#include "hilog/log.h"
#include "netmgr_ext_log_wrapper.h"
#include <string>

using namespace OHOS::NativeRdb;

namespace OHOS {
namespace NetManagerStandard {
std::shared_ptr<NetFirewallDbHelper> NetFirewallDbHelper::instance_;

NetFirewallDbHelper::NetFirewallDbHelper()
{
    firewallDatabase_ = NetFirewallDataBase::GetInstance();
}

std::shared_ptr<NetFirewallDbHelper> NetFirewallDbHelper::GetInstance()
{
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> guard(instanceMutex);
    if (instance_ == nullptr) {
        instance_.reset(new NetFirewallDbHelper());
    }
    return instance_;
}

int32_t NetFirewallDbHelper::FillValuesOfFirewallRule(ValuesBucket &values, const NetFirewallRuleData &rule)
{
    values.Clear();

    values.PutInt("userId", rule.userId);
    values.PutString("ruleName", rule.ruleName);
    values.PutString("ruleDescription", rule.ruleDescription);
    values.PutInt("ruleDirection", static_cast<int32_t>(rule.ruleDirection));
    values.PutInt("ruleAction", static_cast<int32_t>(rule.ruleAction));
    values.PutInt("ruleType", static_cast<int32_t>(rule.ruleType));
    values.PutInt("isEnabled", rule.isEnabled);
    values.PutInt("appUid", rule.appUid);
    if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
        values.PutInt("protocol", static_cast<int32_t>(rule.protocol));
    } else if (rule.ruleType == NetFirewallRuleType::RULE_DNS) {
        values.PutString("primaryDns", rule.dns.primaryDns);
        values.PutString("standbyDns", rule.dns.standbyDns);
    }

    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::FillValuesOfFirewallRule(ValuesBucket &values, const NetFirewallIpRuleData &rule)
{
    values.Clear();

    values.PutInt("ruleId", rule.ruleId);
    values.PutInt("userId", rule.userId);
    values.PutInt("appUid", rule.appUid);
    values.PutInt("locationType", static_cast<int32_t>(rule.locationType));
    values.PutInt("family", rule.family);
    values.PutInt("type", rule.type);
    if (rule.type == SINGLE_IP) {
        values.PutString("address", rule.address);
        values.PutInt("mask", rule.mask);
    } else {
        values.PutString("startIp", rule.startIp);
        values.PutString("endIp", rule.endIp);
    }

    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::FillValuesOfFirewallRule(ValuesBucket &values, const NetFirewallPortRuleData &rule)
{
    values.Clear();

    values.PutInt("ruleId", rule.ruleId);
    values.PutInt("userId", rule.userId);
    values.PutInt("appUid", rule.appUid);
    values.PutInt("locationType", static_cast<int32_t>(rule.locationType));
    values.PutInt("startPort", rule.startPort);
    values.PutInt("endPort", rule.endPort);

    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::FillValuesOfFirewallRule(ValuesBucket &values, const NetFirewallDomainRuleData &rule)
{
    values.Clear();

    values.PutInt("ruleId", rule.ruleId);
    values.PutInt("userId", rule.userId);
    values.PutInt("appUid", rule.appUid);
    values.PutInt("isWildcard", rule.isWildcard);
    values.PutString("domain", rule.domain);

    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::AddFirewallBaseRule(NativeRdb::ValuesBucket &values, const NetFirewallRuleData &rule)
{
    FillValuesOfFirewallRule(values, rule);

    return firewallDatabase_->Insert(values, FIREWALL_TABLE_NAME);
}

int32_t NetFirewallDbHelper::AddFirewallIpRule(ValuesBucket &values, const NetFirewallIpRuleData &rule)
{
    FillValuesOfFirewallRule(values, rule);

    return firewallDatabase_->Insert(values, FIREWALL_TABLE_IP_RULE);
}

int32_t NetFirewallDbHelper::AddFirewallPortRule(ValuesBucket &values, const NetFirewallPortRuleData &rule)
{
    FillValuesOfFirewallRule(values, rule);

    return firewallDatabase_->Insert(values, FIREWALL_TABLE_PORT_RULE);
}

int32_t NetFirewallDbHelper::AddFirewallDomainRule(ValuesBucket &values, const NetFirewallDomainRuleData &rule)
{
    FillValuesOfFirewallRule(values, rule);

    return firewallDatabase_->Insert(values, FIREWALL_TABLE_DOMAIN_RULE);
}

void NetFirewallDbHelper::NetFirewallRule2Data(const NetFirewallRule &rule, NetFirewallRuleData &baseRuleData)
{
    baseRuleData.userId = rule.userId;
    baseRuleData.ruleName = rule.ruleName;
    baseRuleData.ruleDescription = rule.ruleDescription;
    baseRuleData.ruleDirection = rule.ruleDirection;
    baseRuleData.ruleAction = rule.ruleAction;
    baseRuleData.ruleType = rule.ruleType;
    baseRuleData.isEnabled = rule.isEnabled;
    baseRuleData.appUid = rule.appUid;
    baseRuleData.protocol = rule.protocol;
    baseRuleData.dns.primaryDns = rule.dns.primaryDns;
    baseRuleData.dns.standbyDns = rule.dns.standbyDns;
}

void NetFirewallDbHelper::NetFirewallData2Rule(const NetFirewallRuleData &ruleData, NetFirewallRule &rule)
{
    rule.ruleId = ruleData.ruleId;

    rule.ruleDirection = ruleData.ruleDirection;
    rule.ruleName = ruleData.ruleName;
    rule.ruleDescription = ruleData.ruleDescription;
    rule.ruleAction = ruleData.ruleAction;
    rule.ruleType = ruleData.ruleType;
    rule.isEnabled = ruleData.isEnabled;
    rule.appUid = ruleData.appUid;
    rule.protocol = ruleData.protocol;
    rule.dns.primaryDns = ruleData.dns.primaryDns;
    rule.dns.standbyDns = ruleData.dns.standbyDns;
    rule.userId = ruleData.userId;
}

int32_t NetFirewallDbHelper::AddFirewallIpRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule,
    int32_t ruleId, LocationType locationType)
{
    std::vector<NetFirewallIpParam> netFirewallIpParamList =
        locationType == LocationType::SRC_LOCATION ? rule.localIps : rule.remoteIps;
    size_t size = netFirewallIpParamList.size();
    int ret = FIREWALL_OK;
    for (size_t i = 0; i < size; i++) {
        NetFirewallIpRuleData ruledata;
        ruledata.ruleId = ruleId;
        ruledata.userId = rule.userId;
        ruledata.appUid = rule.appUid;
        ruledata.family = netFirewallIpParamList[i].family;
        ruledata.type = netFirewallIpParamList[i].type;
        ruledata.locationType = locationType;
        if (ruledata.type == SINGLE_IP) {
            ruledata.address = netFirewallIpParamList[i].address;
            ruledata.mask = netFirewallIpParamList[i].mask;
        } else {
            ruledata.startIp = netFirewallIpParamList[i].startIp;
            ruledata.endIp = netFirewallIpParamList[i].endIp;
        }
        NETMGR_EXT_LOG_I("Insert ipParam: ruleId=%{public}d startIp=%{public}s", ruleId,
            netFirewallIpParamList[i].startIp.c_str());
        ret = AddFirewallIpRule(values, ruledata);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("Insert error: %{public}d", ret);
            (void)firewallDatabase_->RollBack();
            return ret;
        }
    }
    return ret;
}

int32_t NetFirewallDbHelper::AddFirewallPortRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule,
    int32_t ruleId, LocationType locationType)
{
    std::vector<NetFirewallPortParam> netFirewallPortParamList =
        locationType == LocationType::SRC_LOCATION ? rule.localPorts : rule.remotePorts;
    size_t size = netFirewallPortParamList.size();
    int ret = FIREWALL_OK;
    for (size_t i = 0; i < size; i++) {
        NetFirewallPortRuleData ruledata;
        ruledata.ruleId = ruleId;
        ruledata.userId = rule.userId;
        ruledata.appUid = rule.appUid;
        ruledata.startPort = netFirewallPortParamList[i].startPort;
        ruledata.endPort = netFirewallPortParamList[i].endPort;
        ruledata.locationType = locationType;
        ret = AddFirewallPortRule(values, ruledata);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("Insert error: %{public}d", ret);
            (void)firewallDatabase_->RollBack();
            return ret;
        }
    }
    return ret;
}

int32_t NetFirewallDbHelper::AddFirewallDomainRule(NativeRdb::ValuesBucket &values, const NetFirewallRule &rule,
    int32_t ruleId)
{
    size_t size = rule.domains.size();
    int ret = FIREWALL_OK;
    for (size_t i = 0; i < size; i++) {
        NetFirewallDomainRuleData ruledata;
        ruledata.ruleId = ruleId;
        ruledata.userId = rule.userId;
        ruledata.isWildcard = rule.domains[i].isWildcard;
        ruledata.domain = rule.domains[i].domain;
        ret = AddFirewallDomainRule(values, ruledata);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("Insert error: %{public}d", ret);
            (void)firewallDatabase_->RollBack();
            return ret;
        }
    }
    return ret;
}

int32_t NetFirewallDbHelper::AddFirewallRuleRecord(const NetFirewallRule &rule)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int32_t ret = firewallDatabase_->BeginTransaction();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("BeginTransaction error: %{public}d", ret);
        return ret;
    }
    ValuesBucket values;
    NetFirewallRuleData baseRuleData;
    NetFirewallRule2Data(rule, baseRuleData);
    ret = AddFirewallBaseRule(values, baseRuleData);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Insert error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
        return ret;
    }
    int32_t rowId = ret;
    if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
        ret = AddFirewallIpRule(values, rule, rowId, LocationType::SRC_LOCATION);
        if (ret < FIREWALL_OK) {
            return ret;
        }
        ret = AddFirewallIpRule(values, rule, rowId, LocationType::DST_LOCATION);
        if (ret < FIREWALL_OK) {
            return ret;
        }
        ret = AddFirewallPortRule(values, rule, rowId, LocationType::SRC_LOCATION);
        if (ret < FIREWALL_OK) {
            return ret;
        }
        ret = AddFirewallPortRule(values, rule, rowId, LocationType::DST_LOCATION);
        if (ret < FIREWALL_OK) {
            return ret;
        }
    } else if (rule.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
        ret = AddFirewallDomainRule(values, rule, rowId);
        if (ret < FIREWALL_OK) {
            return ret;
        }
    }
    ret = firewallDatabase_->Commit();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Commit error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
        return ret;
    }
    NETMGR_EXT_LOG_I("add success run time: %{public}llu", GetCurrentMilliseconds() - callStartTime_);
    return rowId;
}

int32_t NetFirewallDbHelper::CheckIfNeedUpdateEx(const std::string &tableName, bool &isUpdate, int32_t ruleId,
    NetFirewallRule &oldRule)
{
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(tableName);
    rdbPredicates.BeginWrap()->EqualTo("ruleId", std::to_string(ruleId))->EndWrap();
    auto resultSet = firewallDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        NETMGR_EXT_LOG_E("Query error");
        (void)firewallDatabase_->RollBack();
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t rowCount = 0;
    if (resultSet->GetRowCount(rowCount) != E_OK) {
        NETMGR_EXT_LOG_E("GetRowCount error");
        (void)firewallDatabase_->RollBack();
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    std::vector<NetFirewallRuleData> rules;
    GetResultRightRecordEx(resultSet, rules);
    isUpdate = rowCount > 0 && !rules.empty();
    if (!rules.empty()) {
        oldRule.ruleId = rules[0].ruleId;
        oldRule.ruleType = rules[0].ruleType;
        oldRule.isEnabled = rules[0].isEnabled;
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::SubTableDelete4UpdateRule(int32_t ruleId)
{
    int32_t changedRows = 0;
    std::string whereClause = { "ruleId = ?" };
    std::vector<std::string> whereArgs = { std::to_string(ruleId) };
    int32_t ret = firewallDatabase_->Delete(FIREWALL_TABLE_IP_RULE, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Delete FIREWALL_TABLE_IP_RULE error: %{public}d", ret);
        return ret;
    }

    changedRows = 0;
    ret = firewallDatabase_->Delete(FIREWALL_TABLE_PORT_RULE, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Delete FIREWALL_TABLE_PORT_RULE error: %{public}d", ret);
        return ret;
    }

    changedRows = 0;
    ret = firewallDatabase_->Delete(FIREWALL_TABLE_DOMAIN_RULE, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Delete FIREWALL_TABLE_DOMAIN_RULE error: %{public}d", ret);
    }
    return ret;
}

int32_t NetFirewallDbHelper::SubTableAdd4UpdateRule(const NetFirewallRule &rule, ValuesBucket &values)
{
    int ret = FIREWALL_OK;
    if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
        ret = SubTableAddIpParam4UpdateRule(rule, LocationType::SRC_LOCATION, values);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("Insert localIps error: %{public}d", ret);
            return ret;
        }
        ret = SubTableAddIpParam4UpdateRule(rule, LocationType::DST_LOCATION, values);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("Insert remoteIps error: %{public}d", ret);
            return ret;
        }
        ret = SubTableAddPortParam4UpdateRule(rule, LocationType::SRC_LOCATION, values);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("Insert localPorts error: %{public}d", ret);
            return ret;
        }
        ret = SubTableAddPortParam4UpdateRule(rule, LocationType::DST_LOCATION, values);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("Insert remotePorts error: %{public}d", ret);
            return ret;
        }
    } else if (rule.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
        size_t size = rule.domains.size();
        for (size_t i = 0; i < size; i++) {
            NetFirewallDomainRuleData ruledata;
            ruledata.ruleId = rule.ruleId;
            ruledata.userId = rule.userId;
            ruledata.isWildcard = rule.domains[i].isWildcard;
            ruledata.domain = rule.domains[i].domain;
            ret = AddFirewallDomainRule(values, ruledata);
            if (ret < FIREWALL_OK) {
                NETMGR_EXT_LOG_E("Insert domains error: %{public}d", ret);
                return ret;
            }
        }
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::SubTableAddIpParam4UpdateRule(const NetFirewallRule &rule, LocationType locationType,
    ValuesBucket &values)
{
    std::vector<NetFirewallIpParam> paramList =
        locationType == LocationType::SRC_LOCATION ? rule.localIps : rule.remoteIps;
    int32_t ret = FIREWALL_OK;
    size_t size = paramList.size();
    for (size_t i = 0; i < size; i++) {
        NetFirewallIpRuleData ruledata;
        ruledata.ruleId = rule.ruleId;
        ruledata.userId = rule.userId;
        ruledata.appUid = rule.appUid;
        ruledata.family = paramList[i].family;
        ruledata.type = paramList[i].type;
        ruledata.locationType = locationType;
        if (ruledata.type == SINGLE_IP) {
            ruledata.address = paramList[i].address;
            ruledata.mask = paramList[i].mask;
        } else {
            ruledata.startIp = paramList[i].startIp;
            ruledata.endIp = paramList[i].endIp;
        }
        ret = AddFirewallIpRule(values, ruledata);
        if (ret < FIREWALL_OK) {
            return ret;
        }
    }
    return ret;
}

int32_t NetFirewallDbHelper::SubTableAddPortParam4UpdateRule(const NetFirewallRule &rule, LocationType locationType,
    ValuesBucket &values)
{
    std::vector<NetFirewallPortParam> paramList =
        locationType == LocationType::SRC_LOCATION ? rule.localPorts : rule.remotePorts;
    int32_t ret = FIREWALL_OK;
    size_t size = paramList.size();
    for (size_t i = 0; i < size; i++) {
        NetFirewallPortRuleData ruledata;
        ruledata.ruleId = rule.ruleId;
        ruledata.userId = rule.userId;
        ruledata.appUid = rule.appUid;
        ruledata.startPort = paramList[i].startPort;
        ruledata.endPort = paramList[i].endPort;
        ruledata.locationType = locationType;
        ret = AddFirewallPortRule(values, ruledata);
        if (ret < FIREWALL_OK) {
            return ret;
        }
    }
    return ret;
}

int32_t NetFirewallDbHelper::UpdateFirewallRuleRecord(const NetFirewallRule &rule)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);

    ValuesBucket values;

    NetFirewallRuleData baseRuleData;
    NetFirewallRule2Data(rule, baseRuleData);

    FillValuesOfFirewallRule(values, baseRuleData);

    int32_t ret = firewallDatabase_->BeginTransaction();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("BeginTransaction error: %{public}d", ret);
        return ret;
    }
    int32_t changedRows = 0;
    ret = firewallDatabase_->Update(FIREWALL_TABLE_NAME, changedRows, values, "ruleId = ?",
        std::vector<std::string> { std::to_string(rule.ruleId) });
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Update error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
        return ret;
    }
    // delete
    ret = SubTableDelete4UpdateRule(rule.ruleId);
    if (ret < FIREWALL_OK) {
        (void)firewallDatabase_->RollBack();
        return ret;
    }
    // insert
    ret = SubTableAdd4UpdateRule(rule, values);
    if (ret < FIREWALL_OK) {
        (void)firewallDatabase_->RollBack();
        return ret;
    }
    ret = firewallDatabase_->Commit();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Commit error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
    }
    return ret;
}

int32_t NetFirewallDbHelper::GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    NetFirewallRuleInfo &table)
{
    std::vector<std::string> columnNames;
    if (resultSet->GetRowCount(table.rowCount) != E_OK || resultSet->GetColumnCount(table.columnCount) != E_OK ||
        resultSet->GetAllColumnNames(columnNames) != E_OK) {
        NETMGR_EXT_LOG_E("get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &columnName = columnNames.at(i);
        if (columnName == "ruleId") {
            table.ruleIdIndex = i;
        }
        if (columnName == "userId") {
            table.userIdIndex = i;
        }
        if (columnName == "ruleName") {
            table.ruleNameIndex = i;
        }
        if (columnName == "ruleDescription") {
            table.ruleDescriptionIndex = i;
        }
        if (columnName == "ruleDirection") {
            table.ruleDirectionIndex = i;
        }
        if (columnName == "ruleAction") {
            table.ruleActionIndex = i;
        }
        if (columnName == "ruleType") {
            table.ruleTypeIndex = i;
        }
        if (columnName == "isEnabled") {
            table.isEnabledIndex = i;
        }
        if (columnName == "appUid") {
            table.appUidIndex = i;
        }
        if (columnName == "protocol") {
            table.protocolIndex = i;
        }

        if (columnName == "primaryDns") {
            table.primaryDnsIndex = i;
        }
        if (columnName == "standbyDns") {
            table.standbyDnsIndex = i;
        }
    }
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::GetResultSetTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    NetInterceptRecordInfo &table)
{
    int32_t rowCount = 0;
    int32_t columnCount = 0;
    std::vector<std::string> columnNames;
    if (resultSet->GetRowCount(rowCount) != E_OK || resultSet->GetColumnCount(columnCount) != E_OK ||
        resultSet->GetAllColumnNames(columnNames) != E_OK) {
        NETMGR_EXT_LOG_E("get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &columnName = columnNames.at(i);
        if (columnName == NET_FIREWALL_RECORD_TIME) {
            table.timeIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_SOURCE_IP) {
            table.sourceIpIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_DEST_IP) {
            table.destIpIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_SOURCE_PORT) {
            table.sourcePortIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_DEST_PORT) {
            table.destPortIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_PROTOCOL) {
            table.protocolIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_UID) {
            table.appUidIndex = i;
        }
        if (columnName == NET_FIREWALL_DOMAIN) {
            table.domainIndex = i;
        }
    }
    table.rowCount = rowCount;
    table.columnCount = columnCount;
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::GetResultSetIpTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    NetFirewallIpRuleInfo &table)
{
    int32_t rowCount = 0;
    int32_t columnCount = 0;
    std::vector<std::string> columnNames;
    if (resultSet->GetRowCount(rowCount) != E_OK || resultSet->GetColumnCount(columnCount) != E_OK ||
        resultSet->GetAllColumnNames(columnNames) != E_OK) {
        NETMGR_EXT_LOG_E("get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }

    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &columnName = columnNames.at(i);
        if (columnName == "id") {
            table.idIndex = i;
        }
        if (columnName == "ruleId") {
            table.ruleIdIndex = i;
        }
        if (columnName == "userId") {
            table.userIdIndex = i;
        }
        if (columnName == "appUid") {
            table.appUidIndex = i;
        }
        if (columnName == "locationType") {
            table.locationTypeIndex = i;
        }
        if (columnName == "family") {
            table.familyIndex = i;
        }
        if (columnName == "type") {
            table.typeIndex = i;
        }
        if (columnName == "address") {
            table.addressIndex = i;
        }
        if (columnName == "mask") {
            table.maskIndex = i;
        }
        if (columnName == "startIp") {
            table.startIpIndex = i;
        }
        if (columnName == "endIp") {
            table.endIpIndex = i;
        }
    }
    table.rowCount = rowCount;
    table.columnCount = columnCount;
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::GetResultSetPortTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    NetFirewallPortRuleInfo &table)
{
    int32_t rowCount = 0;
    int32_t columnCount = 0;
    std::vector<std::string> columnNames;
    if (resultSet->GetRowCount(rowCount) != E_OK || resultSet->GetColumnCount(columnCount) != E_OK ||
        resultSet->GetAllColumnNames(columnNames) != E_OK) {
        NETMGR_EXT_LOG_E("get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }

    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &columnName = columnNames.at(i);
        if (columnName == "id") {
            table.idIndex = i;
        }
        if (columnName == "ruleId") {
            table.ruleIdIndex = i;
        }
        if (columnName == "userId") {
            table.userIdIndex = i;
        }
        if (columnName == "appUid") {
            table.appUidIndex = i;
        }
        if (columnName == "locationType") {
            table.locationTypeIndex = i;
        }
        if (columnName == "startPort") {
            table.startPortIndex = i;
        }
        if (columnName == "endPort") {
            table.endPortIndex = i;
        }
    }
    table.rowCount = rowCount;
    table.columnCount = columnCount;
    NETMGR_EXT_LOG_D("info[%{public}d/%{public}d]: %{public}d/%{public}d", rowCount, columnCount, table.ruleIdIndex,
        table.userIdIndex);
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::GetResultSetDomainTableInfo(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    NetFirewallDomainRuleInfo &table)
{
    int32_t rowCount = 0;
    int32_t columnCount = 0;
    std::vector<std::string> columnNames;
    if (resultSet->GetRowCount(rowCount) != E_OK || resultSet->GetColumnCount(columnCount) != E_OK ||
        resultSet->GetAllColumnNames(columnNames) != E_OK) {
        NETMGR_EXT_LOG_E("get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }

    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &columnName = columnNames.at(i);
        if (columnName == "id") {
            table.idIndex = i;
        }
        if (columnName == "ruleId") {
            table.ruleIdIndex = i;
        }
        if (columnName == "userId") {
            table.userIdIndex = i;
        }
        if (columnName == "appUid") {
            table.appUidIndex = i;
        }
        if (columnName == "isWildcard") {
            table.isWildcardIndex = i;
        }
        if (columnName == "domain") {
            table.domainIndex = i;
        }
    }
    table.rowCount = rowCount;
    table.columnCount = columnCount;
    NETMGR_EXT_LOG_D("info[%{public}d/%{public}d]: %{public}d/%{public}d", rowCount, columnCount, table.ruleIdIndex,
        table.userIdIndex);
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<NetFirewallIpRuleData> &rules)
{
    NetFirewallIpRuleInfo table;
    int32_t ret = GetResultSetIpTableInfo(resultSet, table);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("GetResultSetTableInfo failed");
        return ret;
    }

    bool endFlag = false;
    int32_t locationType = 0;
    for (int32_t i = 0; (i < table.rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GoToRow %{public}d", i);
            break;
        }
        NetFirewallIpRuleData info;
        resultSet->GetInt(table.ruleIdIndex, info.ruleId);
        resultSet->GetInt(table.userIdIndex, info.userId);
        resultSet->GetInt(table.appUidIndex, info.appUid);
        if (resultSet->GetInt(table.locationTypeIndex, locationType) == E_OK) {
            info.locationType = (LocationType)((int)locationType);
        }
        resultSet->GetInt(table.familyIndex, info.family);
        resultSet->GetInt(table.typeIndex, info.type);
        if (info.type == SINGLE_IP) {
            resultSet->GetString(table.addressIndex, info.address);
            resultSet->GetInt(table.maskIndex, info.mask);
        } else {
            resultSet->GetString(table.startIpIndex, info.startIp);

            resultSet->GetString(table.endIpIndex, info.endIp);
        }

        if (info.ruleId > 0) {
            rules.emplace_back(info);
        }

        resultSet->IsEnded(endFlag);
    }
    int32_t position = 0;
    resultSet->GetRowIndex(position);
    resultSet->IsEnded(endFlag);
    NETMGR_EXT_LOG_D("row=%{public}d col=%{public}d pos=%{public}d ret=%{public}zu end=%{public}s", table.rowCount,
        table.columnCount, position, rules.size(), (endFlag ? "yes" : "no"));

    resultSet->Close();
    return rules.size();
}

int32_t NetFirewallDbHelper::GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<NetFirewallPortRuleData> &rules)
{
    NetFirewallPortRuleInfo table;
    int32_t ret = GetResultSetPortTableInfo(resultSet, table);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("GetResultSetTableInfo failed");
        return ret;
    }

    bool endFlag = false;
    int32_t locationType = 0;
    for (int32_t i = 0; (i < table.rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GoToRow %{public}d", i);
            break;
        }
        NetFirewallPortRuleData info;
        resultSet->GetInt(table.ruleIdIndex, info.ruleId);
        resultSet->GetInt(table.userIdIndex, info.userId);
        resultSet->GetInt(table.appUidIndex, info.appUid);
        if (resultSet->GetInt(table.locationTypeIndex, locationType) == E_OK) {
            info.locationType = (LocationType)((int)locationType);
        }
        resultSet->GetInt(table.startPortIndex, info.startPort);
        resultSet->GetInt(table.endPortIndex, info.endPort);
        if (info.ruleId > 0) {
            rules.emplace_back(info);
        }

        resultSet->IsEnded(endFlag);
    }
    int32_t position = 0;
    resultSet->GetRowIndex(position);
    resultSet->IsEnded(endFlag);
    NETMGR_EXT_LOG_D("row=%{public}d col=%{public}d pos=%{public}d ret=%{public}zu end=%{public}s", table.rowCount,
        table.columnCount, position, rules.size(), (endFlag ? "yes" : "no"));

    resultSet->Close();
    return rules.size();
}

int32_t NetFirewallDbHelper::GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<NetFirewallDomainRuleData> &rules)
{
    NetFirewallDomainRuleInfo table;
    int32_t ret = GetResultSetDomainTableInfo(resultSet, table);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("GetResultSetTableInfo failed");
        return ret;
    }

    bool endFlag = false;
    int32_t ruleId = 0;
    int32_t userId = 0;
    int32_t appUid = 0;
    int32_t isWildcard = 0;
    std::string domain = "";

    for (int32_t i = 0; (i < table.rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GoToRow %{public}d", i);
            break;
        }
        NetFirewallDomainRuleData info;
        if (resultSet->GetInt(table.ruleIdIndex, ruleId) == E_OK) {
            info.ruleId = ruleId;
        }
        if (resultSet->GetInt(table.userIdIndex, userId) == E_OK) {
            info.userId = userId;
        }
        if (resultSet->GetInt(table.appUidIndex, appUid) == E_OK) {
            info.appUid = appUid;
        }
        if (resultSet->GetInt(table.isWildcardIndex, isWildcard) == E_OK) {
            info.isWildcard = static_cast<bool>((int)isWildcard);
        }
        if (resultSet->GetString(table.domainIndex, domain) == E_OK) {
            info.domain = domain;
        }

        if (info.ruleId > 0) {
            rules.emplace_back(info);
        }

        resultSet->IsEnded(endFlag);
    }
    int32_t position = 0;
    resultSet->GetRowIndex(position);
    resultSet->IsEnded(endFlag);
    NETMGR_EXT_LOG_D("row=%{public}d col=%{public}d pos=%{public}d ret=%{public}zu end=%{public}s", table.rowCount,
        table.columnCount, position, rules.size(), (endFlag ? "yes" : "no"));

    resultSet->Close();
    return rules.size();
}

void NetFirewallDbHelper::GetRuleDataFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    const NetFirewallRuleInfo &table, NetFirewallRuleData &info)
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
    if (info.ruleType == NetFirewallRuleType::RULE_IP) {
        int protocol = 0;
        if (resultSet->GetInt(table.protocolIndex, protocol) == E_OK) {
            info.protocol = static_cast<NetworkProtocol>(protocol);
        }
    } else if (info.ruleType == NetFirewallRuleType::RULE_DNS) {
        resultSet->GetString(table.primaryDnsIndex, info.dns.primaryDns);
        resultSet->GetString(table.standbyDnsIndex, info.dns.standbyDns);
    }
}

int32_t NetFirewallDbHelper::GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<NetFirewallRuleData> &rules)
{
    NetFirewallRuleInfo table;
    int32_t ret = GetResultSetTableInfo(resultSet, table);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("GetResultSetTableInfo failed");
        return ret;
    }

    bool endFlag = false;
    for (int32_t i = 0; (i < table.rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GoToRow %{public}d", i);
            break;
        }
        NetFirewallRuleData info;
        resultSet->GetInt(table.ruleIdIndex, info.ruleId);
        if (info.ruleId > 0) {
            GetRuleDataFromResultSet(resultSet, table, info);
            rules.emplace_back(info);
        }

        resultSet->IsEnded(endFlag);
    }
    int32_t position = 0;
    resultSet->GetRowIndex(position);
    resultSet->IsEnded(endFlag);
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
    for (int32_t i = 0; (i < table.rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GoToRow %{public}d", i);
            break;
        }
        InterceptRecord info;
        resultSet->GetInt(table.timeIndex, info.time);
        resultSet->GetString(table.sourceIpIndex, info.sourceIp);
        resultSet->GetString(table.destIpIndex, info.destIp);
        resultSet->GetInt(table.sourcePortIndex, info.sourcePort);
        resultSet->GetInt(table.destPortIndex, info.destPort);
        resultSet->GetInt(table.protocolIndex, info.protocol);
        resultSet->GetInt(table.appUidIndex, info.appUid);
        resultSet->GetString(table.domainIndex, info.domain);
        if (info.time > 0) {
            rules.emplace_back(info);
        }
        resultSet->IsEnded(endFlag);
    }
    int32_t position = 0;
    resultSet->GetRowIndex(position);
    resultSet->IsEnded(endFlag);
    NETMGR_EXT_LOG_I("row=%{public}d col=%{public}d pos=%{public}d ret=%{public}zu end=%{public}s", table.rowCount,
        table.columnCount, position, rules.size(), (endFlag ? "yes" : "no"));

    resultSet->Close();
    return rules.size();
}

template <typename T>
int32_t NetFirewallDbHelper::QueryAndGetResult(const NativeRdb::RdbPredicates &rdbPredicates,
    const std::vector<std::string> &columns, std::vector<T> &rules)
{
    int32_t ret = firewallDatabase_->BeginTransaction();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("BeginTransaction error: %{public}d", ret);
        return ret;
    }
    auto resultSet = firewallDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        NETMGR_EXT_LOG_E("Query error");
        (void)firewallDatabase_->RollBack();
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    ret = firewallDatabase_->Commit();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Commit error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
        return ret;
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

int32_t NetFirewallDbHelper::QueryEnabledFirewallRules(int32_t userId, std::vector<NetFirewallRule> &rules)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    NETMGR_EXT_LOG_I("Query detail: userId=%{public}d ", userId);
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->And()->EqualTo("isEnabled", "1")->EndWrap();
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
    return QueryFirewallRuleRecord(rdbPredicates, columns, rules, false);
}

int32_t NetFirewallDbHelper::QueryEnabledDomainOrDnsRules(int32_t userId, std::vector<NetFirewallRule> &rules)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    NETMGR_EXT_LOG_I("QueryEnabledDomainOrDnsRules : userId=%{public}d ", userId);
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()
        ->EqualTo(NET_FIREWALL_USER_ID, std::to_string(userId))
        ->And()
        ->EqualTo(NET_FIREWALL_IS_ENABLED, "1")
        ->And()
        ->NotEqualTo(NET_FIREWALL_RULE_TYPE, "1")
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
        ->EqualTo("ruleId", std::to_string(ruleId))
        ->And()
        ->EqualTo("userId", std::to_string(userId))
        ->EndWrap();

    return QueryFirewallRuleRecord(rdbPredicates, columns, rules);
}

void NetFirewallDbHelper::GetFirewallIpRuleRecord(int32_t ruleId, const std::vector<NetFirewallIpRuleData> &inIpRules,
    std::vector<NetFirewallIpParam> &outIpRules)
{
    outIpRules.clear();
    size_t size = inIpRules.size();
    for (size_t i = 0; i < size; i++) {
        if (inIpRules[i].ruleId == ruleId) {
            NetFirewallIpParam param;
            param.family = inIpRules[i].family;
            param.type = inIpRules[i].type;
            param.address.assign(inIpRules[i].address);
            param.startIp.assign(inIpRules[i].startIp);
            param.endIp.assign(inIpRules[i].endIp);
            param.mask = inIpRules[i].mask;
            outIpRules.emplace_back(param);
        }
    }
}
void NetFirewallDbHelper::GetFirewallPortRuleRecord(int32_t ruleId,
    const std::vector<NetFirewallPortRuleData> &inPortRules, std::vector<NetFirewallPortParam> &outProtRules)
{
    outProtRules.clear();
    size_t size = inPortRules.size();
    for (size_t i = 0; i < size; i++) {
        if (inPortRules[i].ruleId == ruleId) {
            NetFirewallPortParam param;
            param.startPort = inPortRules[i].startPort;
            param.endPort = inPortRules[i].endPort;
            outProtRules.emplace_back(param);
        }
    }
}

void NetFirewallDbHelper::GetFirewallRuleIpSub(const std::vector<NetFirewallIpRuleData> localIps,
    const std::vector<NetFirewallIpRuleData> remoteIps, const std::vector<NetFirewallPortRuleData> &localPorts,
    const std::vector<NetFirewallPortRuleData> &remotePorts, NetFirewallRule &rule)
{
    GetFirewallIpRuleRecord(rule.ruleId, localIps, rule.localIps);
    GetFirewallIpRuleRecord(rule.ruleId, remoteIps, rule.remoteIps);
    GetFirewallPortRuleRecord(rule.ruleId, localPorts, rule.localPorts);
    GetFirewallPortRuleRecord(rule.ruleId, remotePorts, rule.remotePorts);
}

void NetFirewallDbHelper::GetFirewallDomainRuleRecord(int32_t ruleId,
    std::vector<NetFirewallDomainRuleData> &inDomainRules, std::vector<NetFirewallDomainParam> &outDomainRules)
{
    outDomainRules.clear();
    size_t size = inDomainRules.size();
    for (size_t i = 0; i < size; i++) {
        if (inDomainRules[i].ruleId == ruleId) {
            NetFirewallDomainParam param;
            param.isWildcard = inDomainRules[i].isWildcard;
            param.domain.assign(inDomainRules[i].domain);
            outDomainRules.emplace_back(param);
        }
    }
}

int32_t NetFirewallDbHelper::QueryFirewallRuleRecord(const NativeRdb::RdbPredicates &rdbPredicates,
    const std::vector<std::string> &columns, std::vector<NetFirewallRule> &rules, bool isQuerySub)
{
    std::vector<NetFirewallRuleData> ruleDatas;
    int32_t ret = QueryAndGetResult(rdbPredicates, columns, ruleDatas);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("QueryFirewallRuleRecord error.");
        return ret;
    }
    size_t size = ruleDatas.size();
    if (size <= 0) {
        return FIREWALL_OK;
    }
    std::vector<NetFirewallIpRuleData> srcIpRules;
    std::vector<NetFirewallIpRuleData> dstIpRules;
    std::vector<NetFirewallPortRuleData> srcPortRules;
    std::vector<NetFirewallPortRuleData> dstProtRules;
    std::vector<NetFirewallDomainRuleData> domainRules;
    if (isQuerySub) {
        int32_t userId = ruleDatas[0].userId;
        ret = QueryFirewallIpRuleRecord(userId, srcIpRules, dstIpRules);
        if (ret < 0) {
            NETMGR_EXT_LOG_E("QueryFirewallIpRuleRecord error.");
            return ret;
        }
        ret = QueryFirewallPortRuleRecord(userId, srcPortRules, dstProtRules);
        if (ret < 0) {
            NETMGR_EXT_LOG_E("QueryFirewallPortRuleRecord error.");
            return ret;
        }
        ret = QueryFirewallDomainRuleRecord(userId, domainRules);
        if (ret < 0) {
            NETMGR_EXT_LOG_E("QueryFirewallDomainRuleRecord error.");
            return ret;
        }
    }

    for (size_t i = 0; i < size; i++) {
        NetFirewallRule ruleDataParam;
        NetFirewallData2Rule(ruleDatas[i], ruleDataParam);
        if (isQuerySub) {
            if (ruleDataParam.ruleType == NetFirewallRuleType::RULE_IP) {
                GetFirewallRuleIpSub(srcIpRules, dstIpRules, srcPortRules, dstProtRules, ruleDataParam);
            } else if (ruleDataParam.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
                GetFirewallDomainRuleRecord(ruleDatas[i].ruleId, domainRules, ruleDataParam.domains);
            }
        }

        rules.emplace_back(ruleDataParam);
    }
    NETMGR_EXT_LOG_I("QueryFirewallRuleRecord rule size: %{public}d", rules.size());
    return FIREWALL_OK;
}

int32_t NetFirewallDbHelper::QueryFirewallIpRuleRecord(int32_t userId, std::vector<NetFirewallIpRuleData> &srcIpRules,
    std::vector<NetFirewallIpRuleData> &dstIpRules)
{
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_IP_RULE);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();
    std::vector<NetFirewallIpRuleData> ipRuleDatas;
    int32_t ret = QueryAndGetResult(rdbPredicates, columns, ipRuleDatas);
    if (ret > 0) {
        size_t size = ipRuleDatas.size();
        for (size_t i = 0; i < size; i++) {
            NetFirewallIpRuleData ipParam;
            ipParam.ruleId = ipRuleDatas[i].ruleId;
            ipParam.family = ipRuleDatas[i].family;
            ipParam.type = ipRuleDatas[i].type;
            ipParam.address = ipRuleDatas[i].address;
            ipParam.startIp = ipRuleDatas[i].startIp;
            ipParam.endIp = ipRuleDatas[i].endIp;
            ipParam.mask = ipRuleDatas[i].mask;
            if (ipRuleDatas[i].locationType == LocationType::SRC_LOCATION) {
                srcIpRules.emplace_back(ipParam);
            } else {
                dstIpRules.emplace_back(ipParam);
            }
        }
    }
    return ret;
}

int32_t NetFirewallDbHelper::QueryFirewallPortRuleRecord(int32_t userId,
    std::vector<NetFirewallPortRuleData> &srcPortRules, std::vector<NetFirewallPortRuleData> &dstProtRules)
{
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_PORT_RULE);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();
    std::vector<NetFirewallPortRuleData> portRuleDatas;
    int32_t ret = QueryAndGetResult(rdbPredicates, columns, portRuleDatas);
    if (ret > 0) {
        size_t size = portRuleDatas.size();
        for (size_t i = 0; i < size; i++) {
            NetFirewallPortRuleData protParam;
            protParam.ruleId = portRuleDatas[i].ruleId;
            protParam.startPort = portRuleDatas[i].startPort;
            protParam.endPort = portRuleDatas[i].endPort;
            if (portRuleDatas[i].locationType == LocationType::SRC_LOCATION) {
                srcPortRules.emplace_back(protParam);
            } else {
                dstProtRules.emplace_back(protParam);
            }
        }
    }
    return ret;
}

int32_t NetFirewallDbHelper::QueryFirewallDomainRuleRecord(int32_t userId,
    std::vector<NetFirewallDomainRuleData> &domainRules)
{
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_DOMAIN_RULE);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();
    std::vector<NetFirewallDomainRuleData> domainRuleDatas;
    int32_t ret = QueryAndGetResult(rdbPredicates, columns, domainRuleDatas);
    if (ret > 0) {
        domainRuleDatas.swap(domainRules);
    }
    return ret;
}

int32_t NetFirewallDbHelper::DeleteAndNoOtherOperation(const std::string &whereClause,
    const std::vector<std::string> &whereArgs)
{
    int32_t ret = firewallDatabase_->BeginTransaction();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("BeginTransaction error: %{public}d", ret);
        return ret;
    }

    DeleteFirewallRuleRecordByTable(whereClause, whereArgs, FIREWALL_TABLE_NAME);

    DeleteFirewallRuleRecordByTable(whereClause, whereArgs, FIREWALL_TABLE_IP_RULE);

    DeleteFirewallRuleRecordByTable(whereClause, whereArgs, FIREWALL_TABLE_PORT_RULE);

    DeleteFirewallRuleRecordByTable(whereClause, whereArgs, FIREWALL_TABLE_DOMAIN_RULE);

    ret = firewallDatabase_->Commit();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Delete error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
    }
    return ret;
}

int32_t NetFirewallDbHelper::DeleteFirewallRuleRecordByTable(const std::string &whereClause,
    const std::vector<std::string> &whereArgs, const std::string &tableName)
{
    int32_t changedRows = 0;
    int32_t ret = firewallDatabase_->Delete(tableName, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Delete error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
        return ret;
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

bool NetFirewallDbHelper::IsFirewallRuleExits(int32_t ruleId, NetFirewallRule &oldRule)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    bool isExits = false;
    int32_t ret = firewallDatabase_->BeginTransaction();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("BeginTransaction error: %{public}d", ret);
        return isExits;
    }

    ret = CheckIfNeedUpdateEx(FIREWALL_TABLE_NAME, isExits, ruleId, oldRule);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("check if need update error: %{public}d", ret);
        return isExits;
    }

    ret = firewallDatabase_->Commit();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Commit error: %{public}d", ret);
        (void)firewallDatabase_->RollBack();
    }
    return isExits;
}

int32_t NetFirewallDbHelper::QueryFirewallRuleByUserIdCount(int32_t userId, int64_t &rowCount)
{
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();

    return Count(rowCount, rdbPredicates);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleAllCount(int64_t &rowCount)
{
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    return Count(rowCount, rdbPredicates);
}

int32_t NetFirewallDbHelper::QueryFirewallRuleAllDomainCount(int64_t &rowCount)
{
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_DOMAIN_RULE);
    return Count(rowCount, rdbPredicates);
}

int32_t NetFirewallDbHelper::QueryFirewallRule(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<FirewallRulePage> &info)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int64_t rowCount = 0;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();
    firewallDatabase_->Count(rowCount, rdbPredicates);
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
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId));
    if (requestParam->orderType == NetFirewallOrderType::ORDER_ASC) {
        rdbPredicates.OrderByAsc("ruleName");
    } else {
        rdbPredicates.OrderByDesc("ruleName");
    }
    rdbPredicates.Limit((requestParam->page - 1) * requestParam->pageSize, requestParam->pageSize)->EndWrap();
    return QueryFirewallRuleRecord(rdbPredicates, columns, info->data);
}

int32_t NetFirewallDbHelper::Count(int64_t &outValue, const OHOS::NativeRdb::AbsRdbPredicates &predicates)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int32_t ret = firewallDatabase_->BeginTransaction();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("BeginTransaction error: %{public}d", ret);
        return -1;
    }
    ret = firewallDatabase_->Count(outValue, predicates);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Count error");
        return -1;
    }
    ret = firewallDatabase_->Commit();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Commit error: %{public}d", ret);
        return -1;
    }
    return ret;
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
    int32_t ret = firewallDatabase_->BeginTransaction();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("IsDnsRuleExist BeginTransaction error: %{public}d", ret);
        return false;
    }
    std::vector<std::string> columns;
    auto resultSet = firewallDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        NETMGR_EXT_LOG_E("IsDnsRuleExist Query error: %{public}d", ret);
        return false;
    }
    firewallDatabase_->Commit();
    int32_t rowCount = 0;
    resultSet->GetRowCount(rowCount);
    return rowCount > 0;
}

int32_t NetFirewallDbHelper::AddInterceptRecord(const int32_t userId, std::vector<sptr<InterceptRecord>> &records)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    const int MAX_TIME = 8 * 24 * 60 * 60;
    const int MAX_DATA = 999;
    int32_t ret = firewallDatabase_->BeginTransaction();
    // Aging by date, record up to 8 days of data
    std::string whereClause = { "userId = ? AND time < ?" };
    std::vector<std::string> whereArgs = { std::to_string(userId), std::to_string(records.back()->time - MAX_TIME) };
    int32_t changedRows = 0;
    ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);

    int64_t rowCount = 0;
    RdbPredicates rdbPredicates(INTERCEPT_RECORD_TABLE);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();
    firewallDatabase_->Count(rowCount, rdbPredicates);
    // Aging by number, record up to 1000 pieces of data
    size_t size = records.size();
    int64_t offset = MAX_DATA - size;
    if (rowCount >= offset) {
        std::string whereClause = { "userId = ?" };
        std::vector<std::string> whereArgs = { std::to_string(userId), "LIMIT " + std::to_string(rowCount - offset + 1)};
        ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);
    }
    // New data written to the database
    ValuesBucket values;
    for (size_t i = 0; i < size; i++) {
        values.Clear();
        values.PutInt("userId", userId);
        values.PutInt(NET_FIREWALL_RECORD_TIME, records[i]->time);
        values.PutString(NET_FIREWALL_RECORD_SOURCE_IP, records[i]->sourceIp);
        values.PutString(NET_FIREWALL_RECORD_DEST_IP, records[i]->destIp);
        values.PutInt(NET_FIREWALL_RECORD_SOURCE_PORT, records[i]->sourcePort);
        values.PutInt(NET_FIREWALL_RECORD_DEST_PORT, records[i]->destPort);
        values.PutInt(NET_FIREWALL_RECORD_PROTOCOL, records[i]->protocol);
        values.PutInt(NET_FIREWALL_RECORD_UID, records[i]->appUid);
        values.PutString(NET_FIREWALL_DOMAIN, records[i]->domain);

        ret = firewallDatabase_->Insert(values, INTERCEPT_RECORD_TABLE);
        if (ret < FIREWALL_OK) {
            NETMGR_EXT_LOG_E("AddInterceptRecord error: %{public}d", ret);
            return -1;
        }
    }
    return firewallDatabase_->Commit();
}

int32_t NetFirewallDbHelper::DeleteInterceptRecord(const int32_t userId)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int32_t ret = firewallDatabase_->BeginTransaction();
    std::string whereClause = { "userId = ?" };
    std::vector<std::string> whereArgs = { std::to_string(userId) };
    int32_t changedRows = 0;
    ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("DeleteInterceptRecord error: %{public}d", ret);
        return -1;
    }
    return firewallDatabase_->Commit();
}

int32_t NetFirewallDbHelper::QueryInterceptRecord(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int64_t rowCount = 0;
    RdbPredicates rdbPredicates(INTERCEPT_RECORD_TABLE);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();
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
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId));
    if (requestParam->orderType == NetFirewallOrderType::ORDER_ASC) {
        rdbPredicates.OrderByAsc(NET_FIREWALL_RECORD_TIME);
    } else {
        rdbPredicates.OrderByDesc(NET_FIREWALL_RECORD_TIME);
    }
    rdbPredicates.Limit((requestParam->page - 1) * requestParam->pageSize, requestParam->pageSize)->EndWrap();
    return QueryAndGetResult(rdbPredicates, columns, info->data);
}
} // namespace NetManagerStandard
} // namespace OHOS