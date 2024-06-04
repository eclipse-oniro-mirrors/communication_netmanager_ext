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

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_intercept_recorder.h"
#include "netfirewall_db_helper.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t RECORD_CACHE_SIZE = 100;
constexpr int64_t RECORD_TASK_DELAY_TIME_MS = 3 * 60 * 1000;

std::shared_ptr<NetFirewallInterceptRecorder> NetFirewallInterceptRecorder::instance_ = nullptr;

std::shared_ptr<NetFirewallInterceptRecorder> NetFirewallInterceptRecorder::GetInstance()
{
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> guard(instanceMutex);
    if (instance_ == nullptr) {
        instance_.reset(new NetFirewallInterceptRecorder());
    }
    return instance_;
}

NetFirewallInterceptRecorder::NetFirewallInterceptRecorder()
{
    NETMGR_EXT_LOG_I("NetFirewallInterceptRecorder");
}

NetFirewallInterceptRecorder::~NetFirewallInterceptRecorder()
{
    NETMGR_EXT_LOG_I("~NetFirewallInterceptRecorder");
}

int32_t NetFirewallInterceptRecorder::GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
    sptr<InterceptRecordPage> &info)
{
    std::lock_guard<std::shared_mutex> locker(setRecordMutex_);
    info->pageSize = requestParam->pageSize;
    std::shared_ptr<NetFirewallDbHelper> helper = NetFirewallDbHelper::GetInstance();
    int32_t ret = helper->QueryInterceptRecord(userId, requestParam, info);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("GetInterceptRecords error");
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

void NetFirewallInterceptRecorder::SetCurrentUserId(int32_t userId)
{
    NETMGR_EXT_LOG_I("SetCurrentUserId");
    std::lock_guard<std::shared_mutex> locker(setRecordMutex_);
    currentUserId_ = userId;
}

int32_t NetFirewallInterceptRecorder::GetRecordCacheSize()
{
    std::shared_lock<std::shared_mutex> locker(setRecordMutex_);
    return recordCache_.size();
}

void NetFirewallInterceptRecorder::PutRecordCache(sptr<InterceptRecord> record)
{
    std::lock_guard<std::shared_mutex> locker(setRecordMutex_);
    if (record != nullptr) {
        recordCache_.emplace_back(record);
    }
}

void NetFirewallInterceptRecorder::SyncRecordCache()
{
    std::lock_guard<std::shared_mutex> locker(setRecordMutex_);
    if (!recordCache_.empty()) {
        NetFirewallDbHelper::GetInstance()->AddInterceptRecord(currentUserId_, recordCache_);
        recordCache_.clear();
    }
}

int32_t NetFirewallInterceptRecorder::RegisterInterceptCallback()
{
    NETMGR_EXT_LOG_I("RegisterInterceptCallback");
    if (callback_ == nullptr) {
        callback_ = new (std::nothrow) FirewallCallback(shared_from_this());
    }
    return NetsysController::GetInstance().RegisterNetFirewallCallback(callback_);
}

int32_t NetFirewallInterceptRecorder::UnRegisterInterceptCallback()
{
    NETMGR_EXT_LOG_I("UnRegisterInterceptCallback");
    int32_t ret = FIREWALL_SUCCESS;
    if (callback_) {
        ret = NetsysController::GetInstance().UnRegisterNetFirewallCallback(callback_);
        callback_ = nullptr;
    }

    return ret;
}

int32_t NetFirewallInterceptRecorder::FirewallCallback::OnIntercept(sptr<InterceptRecord> &record)
{
    NETMGR_EXT_LOG_I("FirewallCallback::OnIntercept: %{public}s", record->ToString().c_str());
    NetFirewallInterceptRecorder::GetInstance()->PutRecordCache(record);
    if (recordTaskHandle_ != nullptr) {
        ffrtQueue_->cancel(recordTaskHandle_);
        recordTaskHandle_ = nullptr;
    }
    auto callback = [this]() { NetFirewallInterceptRecorder::GetInstance()->SyncRecordCache(); };
    if (NetFirewallInterceptRecorder::GetInstance()->GetRecordCacheSize() < RECORD_CACHE_SIZE) {
        // Write every three minutes when dissatisfied
        recordTaskHandle_ = ffrtQueue_->submit_h(callback, ffrt::task_attr().delay(RECORD_TASK_DELAY_TIME_MS));
    } else {
        ffrtQueue_->submit(callback);
    }
    return FIREWALL_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
