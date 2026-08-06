/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NETTRAFFICFILTER_NFQUEUE_CORE_H
#define NETTRAFFICFILTER_NFQUEUE_CORE_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include "netfirewall_common.h"
#include "application_state_observer_stub.h"
#include "refbase.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
struct QueueInfo {
    uint32_t groupId;
    uint32_t priority;
    uint16_t queueNum;
    std::string bundleName;
    std::string chainNameIn;
    std::string chainNameOut;
    std::string chainNameFwd;
    std::string packetControllerId;
    int fd;
    OHOS::sptr<NfqCtx> nfqHandle;
    OHOS::sptr<NfqQueue> qh;
};
class NetTrafficFilterNFQueueCore {
public:
    static NetTrafficFilterNFQueueCore &GetInstance();
    void Cleanup();
    int32_t CreateQueue(uint32_t groupId, uint32_t priority, uint16_t queueNum, const std::string &bundleName,
        const OHOS::sptr<TrafficFilterConfig>& config);
    int32_t DestroyQueue(uint16_t queueNum);
    int32_t DestroyByBundleName(const std::string &bundleName);

    int32_t AllocateQueueNumber(const std::string &bundleName, uint32_t groupId);
    QueueInfo GetQueueInfo(uint16_t queueNum);

private:
    class TrafficFilterHapObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        explicit TrafficFilterHapObserver(NetTrafficFilterNFQueueCore& nfqueueCore,
                                          const std::string bundleName, int32_t uid)
            : nfQueueCore_(nfqueueCore), bundleName_(bundleName), uid_(uid) {}
        ~TrafficFilterHapObserver() = default;
        void OnProcessDied(const AppExecFwk::ProcessData& processData) override;
    private:
        NetTrafficFilterNFQueueCore& nfQueueCore_;
        std::string bundleName_;
        int32_t uid_;
    };
    NetTrafficFilterNFQueueCore();
    ~NetTrafficFilterNFQueueCore();
    OHOS::sptr<NfqCtx> CreateNFQHandle(const std::string &bundleName);
#ifndef NETMANAGER_TEST
    bool CreateIptables(uint32_t priority, QueueInfo &info, int32_t callingUid, uint32_t groupId);
    void DestroyIptables(const QueueInfo &info);
#endif
    void HandleTrafficFilterObserverRegistration(const std::string bundleName,
        uint16_t queueNum, int32_t uid, int32_t pid);
    void UnregisterTrafficFilterObserver(int32_t uid, const OHOS::sptr<TrafficFilterHapObserver>& observer);
    uint32_t GetPacketCopyMode(const OHOS::sptr<TrafficFilterConfig>& config);
    uint32_t GetPacketCopyLen(const OHOS::sptr<TrafficFilterConfig>& config, uint32_t packetCopyMode);
    uint32_t GetQueueMaxLen(const OHOS::sptr<TrafficFilterConfig>& config);
    uint32_t GetQueueFlags(const OHOS::sptr<TrafficFilterConfig>& config);
    bool ConfigureNFQueue(OHOS::sptr<NfqCtx>& ctx,
        OHOS::sptr<NfqQueue>& qh, const OHOS::sptr<TrafficFilterConfig>& config);
    void UpdateNFQHandleFromBundleName(const std::string &bundleName, const OHOS::sptr<NfqCtx>& nfqHandle);
    OHOS::sptr<NfqCtx> GetNFQHandleFromBundleName(const std::string& bundleName);
    std::map<int32_t, QueueInfo> queues_;

    std::mutex mutex_;
    std::map<int32_t, OHOS::sptr<TrafficFilterHapObserver>> uidToObserverMap_;
    mutable std::mutex observerMutex_;
    uint16_t nextQueueId_ = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETTRAFFICFILTER_NFQUEUE_CORE_H