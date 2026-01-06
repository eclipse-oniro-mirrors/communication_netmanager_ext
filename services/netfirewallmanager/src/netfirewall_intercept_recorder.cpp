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
constexpr int64_t IPC_FLUSH_INTERVAL_MS = 1000;

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
    if (requestParam == nullptr) {
        NETMGR_EXT_LOG_E("GetInterceptRecords requestParam is null");
        return FIREWALL_ERR_PARAMETER_ERROR;
    }
    if (info == nullptr) {
        NETMGR_EXT_LOG_E("GetInterceptRecords info is null");
        return FIREWALL_ERR_INTERNAL;
    }
    std::lock_guard<std::shared_mutex> locker(setRecordMutex_);
    info->pageSize = requestParam->pageSize;
    int32_t ret = NetFirewallDbHelper::GetInstance().QueryInterceptRecord(userId, requestParam, info);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("GetInterceptRecords error");
        return FIREWALL_ERR_INTERNAL;
    }
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallInterceptRecorder::RegisterInterceptRecordsCallback(
    const sptr<INetInterceptRecordCallback>& callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("Callback ptr is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }
    std::lock_guard<std::mutex> locker(interceptRecordCallbackMutex_);
    for (auto it = interceptRecordCallbacks_.begin(); it != interceptRecordCallbacks_.end(); ++it) {
        if ((*it)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            return FIREWALL_ERR_INVALID_PARAMETER;
        }
    }
    interceptRecordCallbacks_.emplace_back(callback);
    return FIREWALL_SUCCESS;
}

int32_t NetFirewallInterceptRecorder::UnregisterInterceptRecordsCallback(
    const sptr<INetInterceptRecordCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("Callback ptr is nullptr.");
        return FIREWALL_ERR_INTERNAL;
    }

    std::lock_guard<std::mutex> locker(interceptRecordCallbackMutex_);
    for (auto it = interceptRecordCallbacks_.begin(); it != interceptRecordCallbacks_.end(); ++it) {
        if ((*it)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            interceptRecordCallbacks_.erase(it);
            return FIREWALL_SUCCESS;
        }
    }
    return FIREWALL_SUCCESS;
}

void NetFirewallInterceptRecorder::SetCurrentUserId(int32_t userId)
{
    NETMGR_EXT_LOG_I("SetCurrentUserId userid = %{public}d", userId);
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

void NetFirewallInterceptRecorder::PutRecordCacheWithoutSkip(sptr<InterceptRecord> &recordWithoutSkip)
{
    std::lock_guard<std::mutex> locker(setRecordWithoutSkipMutex_);
    if (recordWithoutSkip != nullptr) {
        recordCacheWithoutSkip_.emplace_back(recordWithoutSkip);
    }
}

void NetFirewallInterceptRecorder::SyncRecordCache()
{
    std::lock_guard<std::shared_mutex> locker(setRecordMutex_);
    if (!recordCache_.empty()) {
        NetFirewallDbHelper::GetInstance().AddInterceptRecord(currentUserId_, recordCache_);
        recordCache_.clear();
    }
}

int32_t NetFirewallInterceptRecorder::RegisterInterceptCallback()
{
    NETMGR_EXT_LOG_I("RegisterInterceptCallback");
    std::unique_lock<std::shared_mutex> locker(callbackMutex_);
    if (callback_ == nullptr) {
        callback_ = sptr<FirewallCallback>::MakeSptr(shared_from_this());
    }
    locker.unlock();
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

bool NetFirewallInterceptRecorder::ShouldSkipNotify(sptr<InterceptRecord> &record)
{
    if (!record) {
        return true;
    }
    const auto intervalMs = static_cast<decltype(record->time)>(INTERCEPT_BUFF_INTERVAL_MS);
    if (oldRecord_ != nullptr && (record->time - oldRecord_->time) < intervalMs) {
        if (record->localIp == oldRecord_->localIp && record->remoteIp == oldRecord_->remoteIp &&
            record->localPort == oldRecord_->localPort && record->remotePort == oldRecord_->remotePort &&
            record->protocol == oldRecord_->protocol && record->appUid == oldRecord_->appUid) {
            return true;
        }
    }
    oldRecord_ = record;
    return false;
}

int32_t NetFirewallInterceptRecorder::FirewallCallback::OnIntercept(sptr<InterceptRecord> &record)
{
    if (record == nullptr) {
        return FIREWALL_ERR_INTERNAL;
    }
    NETMGR_EXT_LOG_I("FirewallCallback::OnIntercept: time=%{public}lu", record->time);
    ReportInterceptWithoutSkip(record);
    if (!recorder_) {
        return FIREWALL_ERR_INTERNAL;
    }
    if (recorder_->ShouldSkipNotify(record)) {
        return FIREWALL_SUCCESS;
    }
    recorder_->PutRecordCache(record);
    if (recordTaskHandle_ != nullptr) {
        ffrtQueue_->cancel(recordTaskHandle_);
        recordTaskHandle_ = nullptr;
    }
    auto callback = [this]() { recorder_->SyncRecordCache(); };
    if (recorder_->GetRecordCacheSize() < RECORD_CACHE_SIZE) {
        // Write every three minutes when dissatisfied
        recordTaskHandle_ =
            ffrtQueue_->submit_h(callback, ffrt::task_attr().delay(RECORD_TASK_DELAY_TIME_MS).name("OnIntercept"));
    } else {
        ffrtQueue_->submit(callback);
    }
    return FIREWALL_SUCCESS;
}

void NetFirewallInterceptRecorder::FirewallCallback::FlushRecordCacheWithoutSkip()
{
    if (!recorder_) {
        return;
    }
    std::vector<sptr<InterceptRecord>> buffer;
    {
        std::lock_guard<std::mutex> lockerRecordCache(recorder_->setRecordWithoutSkipMutex_);
        buffer.swap(recorder_->recordCacheWithoutSkip_);
    }
    if (buffer.empty()) {
        return;
    }
    std::vector<sptr<INetInterceptRecordCallback>> interceptRecordCallback;
    {
        std::lock_guard<std::mutex> lockerCallback(recorder_->interceptRecordCallbackMutex_);
        interceptRecordCallback.swap(recorder_->interceptRecordCallbacks_);
    }
    for (const auto &cb : interceptRecordCallback) {
        if (!cb) {
            continue;
        }
        for (const auto &item : buffer) {
            cb->OnInterceptRecord(item);
        }
    }
}

void NetFirewallInterceptRecorder::FirewallCallback::ReportInterceptWithoutSkip(sptr<InterceptRecord> &record)
{
    if (!recorder_) {
        return;
    }
    if (recorder_->interceptRecordCallbacks_.empty()) {
        return;
    }
    recorder_->PutRecordCacheWithoutSkip(record);
    if (recordWithoutSkipTaskHandle_ != nullptr) {
        ffrtQueue_->cancel(recordWithoutSkipTaskHandle_);
        recordWithoutSkipTaskHandle_ = nullptr;
    }

    auto flushCallback = [this]() { FlushRecordCacheWithoutSkip(); };

    if (recorder_->recordCacheWithoutSkip_.size() >= RECORD_CACHE_SIZE) {
        FlushRecordCacheWithoutSkip();
    } else {
        recordWithoutSkipTaskHandle_ = ffrtQueue_->submit_h(
            flushCallback, ffrt::task_attr().delay(IPC_FLUSH_INTERVAL_MS).name("ReportInterceptWithoutSkipFlush"));
    }
    return;
}
} // namespace NetManagerStandard
} // namespace OHOS
