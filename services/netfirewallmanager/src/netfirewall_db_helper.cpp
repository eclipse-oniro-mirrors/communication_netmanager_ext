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

int32_t NetFirewallDbHelper::AddInterceptRecord(const int32_t userId, std::vector<sptr<InterceptRecord>> &records)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    const int MAX_TIME = 8 * 24 * 60 * 60;
    const int MAX_DATA = 9999;
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
    // Aging by number, record up to 10000 pieces of data
    size_t size = records.size();
    int64_t offset = MAX_DATA - size;
    if (rowCount >= offset) {
        std::string whereClause = { "userId = ?" };
        std::vector<std::string> whereArgs = { std::to_string(userId), "LIMIT " + std::to_string(rowCount - offset + 1),
            "OFFSET " + std::to_string(offset) };
        ret = firewallDatabase_->Delete(INTERCEPT_RECORD_TABLE, changedRows, whereClause, whereArgs);
    }
    // Write new data
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