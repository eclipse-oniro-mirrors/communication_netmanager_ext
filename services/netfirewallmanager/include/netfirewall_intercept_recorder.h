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

#ifndef NET_FIREWALL_INTERCEPT_RECORDER_H
#define NET_FIREWALL_INTERCEPT_RECORDER_H

#include <string>
#include <shared_mutex>

#include "ffrt.h"
#include "netfirewall_common.h"
#include "netfirewall_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallInterceptRecorder : public std::enable_shared_from_this<NetFirewallInterceptRecorder> {
public:
    // Firewall interception log callback
    class FirewallCallback : public OHOS::NetsysNative::NetFirewallCallbackStub {
    public:
        FirewallCallback(std::shared_ptr<NetFirewallInterceptRecorder> recorder) : recorder_(recorder)
        {
            ffrtQueue_ =
                std::make_shared<ffrt::queue>("FirewallCallbackQueue", ffrt::queue_attr().qos(ffrt::qos_utility));
        }

        ~FirewallCallback()
        {
            ffrtQueue_.reset();
            ffrtQueue_ = nullptr;
        }
        virtual int32_t OnIntercept(sptr<InterceptRecord> &record) override;

    private:
        std::shared_ptr<NetFirewallInterceptRecorder> recorder_ = nullptr;
        ffrt::task_handle recordTaskHandle_;
        std::shared_ptr<ffrt::queue> ffrtQueue_;
    };

public:
    static std::shared_ptr<NetFirewallInterceptRecorder> GetInstance();
    NetFirewallInterceptRecorder();
    ~NetFirewallInterceptRecorder();

    /**
     * Set current forground user Id
     *
     * @param userId User id
     */
    void SetCurrentUserId(int32_t userId);

    /**
     * Get all interception records
     *
     * @param userId User ID
     * @param requestParam Paging in parameter information
     * @param info Paging data information
     * @return Returns 0 success. Otherwise fail
     */
    int32_t GetInterceptRecords(const int32_t userId, const sptr<RequestParam> &requestParam,
        sptr<InterceptRecordPage> &info);

    /**
     * sync interception records and insert to db
     */
    void SyncRecordCache();

    /**
     * add interception records in cache
     *
     * @param reocrd record object
     */
    void PutRecordCache(sptr<InterceptRecord> reocrd);

    /**
     * get interception records size in cache
     *
     * @return Returns reocrds size
     */
    int32_t GetRecordCacheSize();

    /**
     * Register callback for recevie intercept event
     *
     * @param callback implement of INetFirewallCallback
     * @return 0 if success or -1 if an error occurred
     */
    int32_t RegisterInterceptCallback();

    /**
     * Unregister callback for recevie intercept event
     *
     * @param callback register callback for recevie intercept event
     * @return 0 if success or -1 if an error occurred
     */
    int32_t UnRegisterInterceptCallback();

private:
    std::shared_mutex setRecordMutex_;
    std::atomic<int32_t> currentUserId_ = 0;
    std::vector<sptr<InterceptRecord>> recordCache_;
    sptr<OHOS::NetsysNative::INetFirewallCallback> callback_ = nullptr;
    static std::shared_ptr<NetFirewallInterceptRecorder> instance_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_INTERCEPT_RECORDER_H */
