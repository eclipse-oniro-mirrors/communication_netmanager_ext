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

#include <string>

#include "hilog/log.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_db_helper.h"

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
    int32_t dstIpRet = FIREWALL_OK;
    int32_t srcPortRet = FIREWALL_OK;
    int32_t dstPortRet = FIREWALL_OK;
    if (rule.ruleType == NetFirewallRuleType::RULE_IP) {
        ret = AddFirewallIpRule(values, rule, rowId, LocationType::SRC_LOCATION);
        dstIpRet = AddFirewallIpRule(values, rule, rowId, LocationType::DST_LOCATION);
        srcPortRet = AddFirewallPortRule(values, rule, rowId, LocationType::SRC_LOCATION);
        dstPortRet = AddFirewallPortRule(values, rule, rowId, LocationType::DST_LOCATION);
    } else if (rule.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
        ret = AddFirewallDomainRule(values, rule, rowId);
    }
    if (ret < FIREWALL_OK || dstIpRet < FIREWALL_OK || srcPortRet < FIREWALL_OK || dstPortRet < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Add sub rule error");
        (void)firewallDatabase_->RollBack();
        return FIREWALL_FAILURE;
    }
    ret = firewallDatabase_->Commit();
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Commit error: %{public}d", ret);
        return ret;
    }
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
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }
    int32_t rowCount = 0;
    if (resultSet->GetRowCount(rowCount) != E_OK) {
        NETMGR_EXT_LOG_E("GetRowCount error");
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
        return FIREWALL_OK;
    }
    if (rule.ruleType == NetFirewallRuleType::RULE_DOMAIN) {
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
    return firewallDatabase_->Commit();
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
        if (columnName == NET_FIREWALL_RECORD_LOCAL_IP) {
            table.localIpIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_REMOTE_IP) {
            table.remoteIpIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_LOCAL_PORT) {
            table.localPortIndex = i;
        }
        if (columnName == NET_FIREWALL_RECORD_REMOTE_PORT) {
            table.remotePortIndex = i;
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
        NETMGR_EXT_LOG_E("GetResultSetIpTableInfo get table info failed");
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
        NETMGR_EXT_LOG_E("GetResultSetPortTableInfo get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }

    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &name = columnNames.at(i);
        if (name == "id") {
            table.idIndex = i;
        }
        if (name == "ruleId") {
            table.ruleIdIndex = i;
        }
        if (name == "userId") {
            table.userIdIndex = i;
        }
        if (name == "appUid") {
            table.appUidIndex = i;
        }
        if (name == "locationType") {
            table.locationTypeIndex = i;
        }
        if (name == "startPort") {
            table.startPortIndex = i;
        }
        if (name == "endPort") {
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
        NETMGR_EXT_LOG_E("GetResultSetDomainTableInfo get table info failed");
        return FIREWALL_RDB_EXECUTE_FAILTURE;
    }

    int32_t columnNamesCount = static_cast<int32_t>(columnNames.size());
    for (int32_t i = 0; i < columnNamesCount; i++) {
        std::string &column = columnNames.at(i);
        if (column == "id") {
            table.idIndex = i;
        }
        if (column == "ruleId") {
            table.ruleIdIndex = i;
        }
        if (column == "userId") {
            table.userIdIndex = i;
        }
        if (column == "appUid") {
            table.appUidIndex = i;
        }
        if (column == "isWildcard") {
            table.isWildcardIndex = i;
        }
        if (column == "domain") {
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
    resultSet->Close();
    return rules.size();
}

int32_t NetFirewallDbHelper::GetResultRightRecordEx(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<NetFirewallPortRuleData> &rules)
{
    NetFirewallPortRuleInfo table;
    int32_t ret = GetResultSetPortTableInfo(resultSet, table);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("GetResultRightRecordEx GetResultSetTableInfo failed");
        return ret;
    }

    bool endFlag = false;
    int locationType = 0;
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
            info.locationType = (LocationType)(locationType);
        }
        resultSet->GetInt(table.startPortIndex, info.startPort);
        resultSet->GetInt(table.endPortIndex, info.endPort);
        if (info.ruleId > 0) {
            rules.emplace_back(info);
        }

        resultSet->IsEnded(endFlag);
    }
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

    bool isEnd = false;
    int32_t isWildcard = 0;

    for (int32_t i = 0; (i < table.rowCount) && !isEnd; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            NETMGR_EXT_LOG_E("GoToRow %{public}d", i);
            break;
        }
        NetFirewallDomainRuleData info;
        resultSet->GetInt(table.ruleIdIndex, info.ruleId);
        resultSet->GetInt(table.userIdIndex, info.userId);
        resultSet->GetInt(table.appUidIndex, info.appUid);
        if (resultSet->GetInt(table.isWildcardIndex, isWildcard) == E_OK) {
            info.isWildcard = static_cast<bool>(isWildcard);
        }
        resultSet->GetString(table.domainIndex, info.domain);
        if (info.ruleId > 0) {
            rules.emplace_back(info);
        }
        resultSet->IsEnded(isEnd);
    }
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
            NETMGR_EXT_LOG_E("GetResultRightRecordEx GoToRow %{public}d", i);
            break;
        }
        InterceptRecord info;
        resultSet->GetInt(table.timeIndex, info.time);
        resultSet->GetString(table.localIpIndex, info.localIp);
        resultSet->GetString(table.remoteIpIndex, info.remoteIp);
        resultSet->GetInt(table.localPortIndex, info.localPort);
        resultSet->GetInt(table.remotePortIndex, info.remotePort);
        resultSet->GetInt(table.protocolIndex, info.protocol);
        resultSet->GetInt(table.appUidIndex, info.appUid);
        resultSet->GetString(table.domainIndex, info.domain);
        if (info.time > 0) {
            rules.emplace_back(info);
        }
        resultSet->IsEnded(endFlag);
    }
    int32_t index = 0;
    resultSet->GetRowIndex(index);
    resultSet->IsEnded(endFlag);
    NETMGR_EXT_LOG_I("row=%{public}d col=%{public}d pos=%{public}d ret=%{public}zu end=%{public}s", table.rowCount,
        table.columnCount, index, rules.size(), (endFlag ? "yes" : "no"));

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

int32_t NetFirewallDbHelper::QueryEnabledFirewallRules(std::vector<NetFirewallRule> &rules,
    NetFirewallRuleType type)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    NETMGR_EXT_LOG_I("Query detail: all userId");
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(FIREWALL_TABLE_NAME);
    rdbPredicates.BeginWrap()
        ->EqualTo(NET_FIREWALL_IS_ENABLED, "1");
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
    return QueryFirewallRuleRecord(rdbPredicates, columns, rules, false);
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
            outIpRules.emplace_back(std::move(param));
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
            outProtRules.emplace_back(std::move(param));
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
            outDomainRules.emplace_back(std::move(param));
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
        int32_t portRet = QueryFirewallPortRuleRecord(userId, srcPortRules, dstProtRules);
        int32_t domainRet = QueryFirewallDomainRuleRecord(userId, domainRules);
        if (ret < 0 || portRet < 0 || domainRet < 0) {
            NETMGR_EXT_LOG_E("QueryFirewallRuleRecord sub query error.");
            return FIREWALL_FAILURE;
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

        rules.emplace_back(std::move(ruleDataParam));
    }
    NETMGR_EXT_LOG_I("QueryFirewallRuleRecord rule size: %{public}zu", rules.size());
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
    if (ret <= 0) {
        return ret;
    }
    for (auto &ipParam : ipRuleDatas) {
        if (ipParam.locationType == LocationType::SRC_LOCATION) {
            srcIpRules.emplace_back(ipParam);
        } else {
            dstIpRules.emplace_back(ipParam);
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
    if (ret <= 0) {
        return ret;
    }
    for (auto &portParam : portRuleDatas) {
        if (portParam.locationType == LocationType::SRC_LOCATION) {
            srcPortRules.emplace_back(portParam);
        } else {
            dstProtRules.emplace_back(portParam);
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
    int32_t changedRows = 0;
    ret = firewallDatabase_->Delete(FIREWALL_TABLE_NAME, changedRows, whereClause, whereArgs);
    changedRows = 0;
    int32_t ipRet = firewallDatabase_->Delete(FIREWALL_TABLE_IP_RULE, changedRows, whereClause, whereArgs);
    changedRows = 0;
    int32_t portRet = firewallDatabase_->Delete(FIREWALL_TABLE_PORT_RULE, changedRows, whereClause, whereArgs);
    changedRows = 0;
    int32_t domainRet = firewallDatabase_->Delete(FIREWALL_TABLE_DOMAIN_RULE, changedRows, whereClause, whereArgs);
    if (ret < FIREWALL_OK || ipRet < FIREWALL_OK || portRet < FIREWALL_OK || domainRet < FIREWALL_OK) {
        (void)firewallDatabase_->RollBack();
        return FIREWALL_FAILURE;
    }
    return firewallDatabase_->Commit();
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
    int32_t ret = CheckIfNeedUpdateEx(FIREWALL_TABLE_NAME, isExits, ruleId, oldRule);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("check if need update error: %{public}d", ret);
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
    int32_t ret = firewallDatabase_->Count(outValue, predicates);
    if (ret < FIREWALL_OK) {
        NETMGR_EXT_LOG_E("Count error");
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

    int64_t rowCount = 0;
    RdbPredicates rdbPredicates(INTERCEPT_RECORD_TABLE);
    rdbPredicates.BeginWrap()->EqualTo("userId", std::to_string(userId))->EndWrap();
    firewallDatabase_->Count(rowCount, rdbPredicates);
    // Aging by number, record up to 1000 pieces of data
    size_t size = records.size();
    int64_t offset = RECORD_MAX_DATA_NUM - 1 - size;
    if (rowCount >= offset) {
        std::string whereClause = { "userId = ?" };
        std::vector<std::string> whereArgs = { std::to_string(userId),
            "LIMIT " + std::to_string(rowCount - offset + 1) };
        ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);
    }
    // New data written to the database
    ValuesBucket values;
    for (size_t i = 0; i < size; i++) {
        values.Clear();
        values.PutInt("userId", userId);
        values.PutInt(NET_FIREWALL_RECORD_TIME, records[i]->time);
        values.PutString(NET_FIREWALL_RECORD_LOCAL_IP, records[i]->localIp);
        values.PutString(NET_FIREWALL_RECORD_REMOTE_IP, records[i]->remoteIp);
        values.PutInt(NET_FIREWALL_RECORD_LOCAL_PORT, records[i]->localPort);
        values.PutInt(NET_FIREWALL_RECORD_REMOTE_PORT, records[i]->remotePort);
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