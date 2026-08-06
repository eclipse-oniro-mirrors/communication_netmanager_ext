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

#include "nettrafficfilter_nfqueue_core.h"
#include "nettrafficfilter_iptables_command_builder.h"
#include "net_manager_constants.h"
#include "nettrafficfilter_packetrule_manager.h"
#include "ipc_skeleton.h"
#include "singleton.h"
#include "app_mgr_client.h"
#include "netmanager_ext_log.h"

using namespace OHOS::NetManagerStandard;
using namespace OHOS::NetsysNative;
constexpr uint32_t DEFAULT_PACKET_COPY_LEN = 0xFFFF;
constexpr uint32_t DEFAULT_MAX_QUEUE_LEN = 1024;
constexpr uint32_t COPY_MODE_META = 0;
constexpr uint32_t COPY_MODE_HEADER = 1;
constexpr uint32_t COPY_MODE_FULL = 2;
constexpr uint32_t COPY_MODE_MAXLEN = 3;
constexpr int32_t NFQ_MAX_QUEUES = 64;
constexpr uint32_t PACKET_COPY_HEADER_LEN = 256;

NetTrafficFilterNFQueueCore::NetTrafficFilterNFQueueCore() {}

NetTrafficFilterNFQueueCore::~NetTrafficFilterNFQueueCore()
{
    Cleanup();
}

NetTrafficFilterNFQueueCore &NetTrafficFilterNFQueueCore::GetInstance()
{
    static NetTrafficFilterNFQueueCore instance;
    return instance;
}

OHOS::sptr<NfqCtx> NetTrafficFilterNFQueueCore::GetNFQHandleFromBundleName(const std::string &bundleName)
{
    for (const auto it : queues_) {
        if (it.second.bundleName == bundleName) {
            return it.second.nfqHandle;
        }
    }
    return nullptr;
}

OHOS::sptr<NfqCtx> NetTrafficFilterNFQueueCore::CreateNFQHandle(const std::string &bundleName)
{
    OHOS::sptr<NfqCtx> nfqHandle = GetNFQHandleFromBundleName(bundleName);
    if (nfqHandle != nullptr) {
        return nfqHandle;
    }
    nfqHandle = NetsysController::GetInstance().NfqOpen();
    if (nfqHandle == nullptr) {
        return nullptr;
    }
    auto rollback = [nfqHandle](bool unbindV4) mutable {
        if (unbindV4) {
            NetsysController::GetInstance().NfqUnbindPf(nfqHandle, AF_INET);
        }
        NetsysController::GetInstance().NfqClose(nfqHandle);
        return nullptr;
    };
    NetsysController::GetInstance().NfqUnbindPf(nfqHandle, AF_INET);
    NetsysController::GetInstance().NfqUnbindPf(nfqHandle, AF_INET6);
    if (NetsysController::GetInstance().NfqBindPf(nfqHandle, AF_INET) < 0
        || NetsysController::GetInstance().NfqBindPf(nfqHandle, AF_INET6) < 0) {
        return rollback(NetsysController::GetInstance().NfqBindPf(nfqHandle, AF_INET) >= 0);
    }
    return nfqHandle;
}

int32_t NetTrafficFilterNFQueueCore::AllocateQueueNumber(const std::string &bundleName, uint32_t groupId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto it : queues_) {
        if (it.second.groupId == groupId && it.second.bundleName == bundleName) {
            return -1;
        }
    }
    uint16_t newQueueId = nextQueueId_ % UINT16_MAX + 1;
    uint16_t counter = 0;
    while (queues_.find(newQueueId) != queues_.end()) {
        newQueueId = newQueueId % UINT16_MAX + 1;
        counter++;
        if (counter >= UINT16_MAX) {
            return -1;
        }
    }
    nextQueueId_ = newQueueId % (UINT16_MAX + 1);
    return newQueueId;
}

QueueInfo NetTrafficFilterNFQueueCore::GetQueueInfo(uint16_t queueNum)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queues_.find(queueNum) != queues_.end()) {
        return queues_[queueNum];
    }
    return QueueInfo{0, 0, 0, "", "", "", "", "", -1, nullptr, nullptr};
}

void NetTrafficFilterNFQueueCore::Cleanup()
{
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (auto it : uidToObserverMap_) {
            if (it.second) {
                Singleton<AppExecFwk::AppMgrClient>::GetInstance().UnregisterApplicationStateObserver(it.second);
            }
        }
        uidToObserverMap_.clear();
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::set<std::string> bundleNames;
    for (auto &pair : queues_) {
        bundleNames.insert(pair.second.bundleName);
        NetsysController::GetInstance().NfqQueueDestroy(pair.second.nfqHandle, pair.second.qh);
    }
    for (auto it : bundleNames) {
        OHOS::sptr<NfqCtx> nfqHandle = GetNFQHandleFromBundleName(it);
        if (nfqHandle) {
            NetsysController::GetInstance().NfqClose(nfqHandle);
        }
    }
    queues_.clear();
    nextQueueId_ = 0;
}

uint32_t NetTrafficFilterNFQueueCore::GetPacketCopyMode(const OHOS::sptr<TrafficFilterConfig>& config)
{
    if (config != nullptr &&
        config->packetCopyMode_ >= COPY_MODE_META &&
        config->packetCopyMode_ <= COPY_MODE_MAXLEN) {
        return config->packetCopyMode_;
    }
    return COPY_MODE_FULL;
}

uint32_t NetTrafficFilterNFQueueCore::GetPacketCopyLen(const OHOS::sptr<TrafficFilterConfig>& config,
    uint32_t packetCopyMode)
{
    if (config == nullptr) {
        return DEFAULT_PACKET_COPY_LEN;
    }
    if (packetCopyMode == COPY_MODE_HEADER ||
        packetCopyMode == COPY_MODE_MAXLEN) {
        return config->packetCopyLen_;
    }
    return DEFAULT_PACKET_COPY_LEN;
}

uint32_t NetTrafficFilterNFQueueCore::GetQueueMaxLen(const OHOS::sptr<TrafficFilterConfig>& config)
{
    if (config != nullptr && config->nfqueueMaxlen_ > 0) {
        return config->nfqueueMaxlen_;
    }
    return DEFAULT_MAX_QUEUE_LEN;
}

uint32_t NetTrafficFilterNFQueueCore::GetQueueFlags(const OHOS::sptr<TrafficFilterConfig>& config)
{
    if (config != nullptr) {
        return config->nfqueueFlags_;
    }
    return 1;
}

bool NetTrafficFilterNFQueueCore::ConfigureNFQueue(OHOS::sptr<NfqCtx> &ctx,
    OHOS::sptr<NfqQueue> &qh, const OHOS::sptr<TrafficFilterConfig>& config)
{
    uint32_t packetCopyMode = GetPacketCopyMode(config);
    uint32_t packetCopyLen = GetPacketCopyLen(config, packetCopyMode);
    uint32_t maxLen = GetQueueMaxLen(config);
    uint32_t flags = GetQueueFlags(config);
    int ret = NetsysController::GetInstance().NfqQueueSetFlag(ctx, qh, 1, flags);
    if (ret != 0) {
        return false;
    }
    switch (packetCopyMode) {
        case COPY_MODE_META:
            ret = NetsysController::GetInstance().NfqQueueSetMode(ctx, qh, NetsysNative::NFQ_COPY_META, 0);
            break;
        case COPY_MODE_HEADER:
            ret = NetsysController::GetInstance().NfqQueueSetMode(ctx, qh,
                NetsysNative::NFQ_COPY_PACKET, PACKET_COPY_HEADER_LEN);
            break;
        case COPY_MODE_FULL:
            ret = NetsysController::GetInstance().NfqQueueSetMode(ctx, qh,
                NetsysNative::NFQ_COPY_PACKET, DEFAULT_PACKET_COPY_LEN);
            break;
        case COPY_MODE_MAXLEN:
            ret = NetsysController::GetInstance().NfqQueueSetMode(ctx, qh,
                NetsysNative::NFQ_COPY_PACKET, packetCopyLen);
            break;
        default:
            ret = NetsysController::GetInstance().NfqQueueSetMode(ctx, qh,
                NetsysNative::NFQ_COPY_PACKET, DEFAULT_PACKET_COPY_LEN);
            break;
    }
    if (ret != 0) {
        return false;
    }
    ret = NetsysController::GetInstance().NfqQueueSetMaxLen(ctx, qh, maxLen);
    if (ret != 0) {
        return false;
    }
    return true;
}

int32_t NetTrafficFilterNFQueueCore::CreateQueue(uint32_t groupId, uint32_t priority, uint16_t queueNum,
                                                 const std::string &bundleName,
                                                 const OHOS::sptr<TrafficFilterConfig>& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    OHOS::sptr<NfqCtx> nfqHandle = CreateNFQHandle(bundleName);
    if (nfqHandle == nullptr) {
        return TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }
    OHOS::sptr<NfqQueue> qh = NetsysController::GetInstance().NfqQueueCreate(nfqHandle, queueNum);
    if (qh == nullptr) {
        NetsysController::GetInstance().NfqClose(nfqHandle);
        return TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }
    if (!ConfigureNFQueue(nfqHandle, qh, config)) {
        NetsysController::GetInstance().NfqQueueDestroy(nfqHandle, qh);
        return TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    QueueInfo info{groupId, priority, queueNum, bundleName, "", "", "", "", nfqHandle->fd, nfqHandle, qh};
    info.packetControllerId = bundleName + ":" + std::to_string(queueNum);
#ifndef NETMANAGER_TEST
    if (!CreateIptables(priority, info, callingUid, groupId)) {
        DestroyIptables(info);
        NetsysController::GetInstance().NfqQueueDestroy(nfqHandle, qh);
        return TRAFFICFILTER_ERROR_NFQUEUE_ERROR;
    }
#endif
    queues_[queueNum] = info;
    HandleTrafficFilterObserverRegistration(bundleName, queueNum, callingUid, callingPid);
    return TRAFFICFILTER_OK;
}

void NetTrafficFilterNFQueueCore::HandleTrafficFilterObserverRegistration(
    const std::string bundleName, uint16_t queueNum, int32_t uid, int32_t pid)
{
    if (bundleName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (uidToObserverMap_.find(uid) != uidToObserverMap_.end()) {
        return;
    }
    std::vector<std::string> list = {bundleName, bundleName + ":trafficfilter"};
    OHOS::sptr<TrafficFilterHapObserver> observer =
        new TrafficFilterHapObserver(*this, bundleName, uid);
    Singleton<AppExecFwk::AppMgrClient>::GetInstance().RegisterApplicationStateObserver(observer, list);
    uidToObserverMap_[uid] = observer;
}

void NetTrafficFilterNFQueueCore::UnregisterTrafficFilterObserver(
    int32_t uid, const OHOS::sptr<TrafficFilterHapObserver>& observer)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = uidToObserverMap_.find(uid);
    if (it != uidToObserverMap_.end() && it->second == observer) {
        uidToObserverMap_.erase(it);
        Singleton<AppExecFwk::AppMgrClient>::GetInstance().UnregisterApplicationStateObserver(observer);
    }
}

void NetTrafficFilterNFQueueCore::TrafficFilterHapObserver::OnProcessDied(
    const AppExecFwk::ProcessData& processData)
{
    if (processData.uid != uid_) {
        return;
    }
    nfQueueCore_.DestroyByBundleName(bundleName_);
    nfQueueCore_.UnregisterTrafficFilterObserver(processData.uid, this);
}

#ifndef NETMANAGER_TEST
bool NetTrafficFilterNFQueueCore::CreateIptables(uint32_t priority, QueueInfo &info,
    int32_t callingUid, uint32_t groupId)
{
    std::string baseName = NetTrafficFilterIptablesCommandBuilder::GenerateChainName(callingUid, groupId);
    info.chainNameIn  = baseName + "_IN";
    info.chainNameOut = baseName + "_OUT";
    info.chainNameFwd = baseName + "_FWD";
    for (const auto& chain : {info.chainNameIn, info.chainNameOut, info.chainNameFwd}) {
        std::string createCmd = NetTrafficFilterIptablesCommandBuilder::BuildCreateChainCommand(chain,
            IptablesName::FILTER);
        int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
            createCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
        if (ret != TRAFFICFILTER_OK) {
            return false;
        }
    }
    uint32_t pos = 1;
    for (const auto& pair : queues_) {
        if (pair.second.priority < priority) {
            pos++;
        }
    }
    std::map<TrafficFilterHookPoint, std::string> hookToChain = {
        {TrafficFilterHookPoint::HOOK_INPUT,    info.chainNameIn},
        {TrafficFilterHookPoint::HOOK_OUTPUT,   info.chainNameOut},
        {TrafficFilterHookPoint::HOOK_FORWARD,  info.chainNameFwd},
    };
    for (const auto& [hook, chain] : hookToChain) {
        std::string hookName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hook);
        std::string jumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildInsertJumpToChainCommand(
            hookName, chain, pos, IptablesName::FILTER);
        int32_t ret = NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
            jumpCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
        if (ret != TRAFFICFILTER_OK) {
            return false;
        }
    }
    return true;
}

void NetTrafficFilterNFQueueCore::DestroyIptables(const QueueInfo &info)
{
    std::map<TrafficFilterHookPoint, std::string> hookToChain = {
        {TrafficFilterHookPoint::HOOK_INPUT,    info.chainNameIn},
        {TrafficFilterHookPoint::HOOK_OUTPUT,   info.chainNameOut},
        {TrafficFilterHookPoint::HOOK_FORWARD,  info.chainNameFwd},
    };
    for (const auto& [hook, chain] : hookToChain) {
        std::string hookName = NetTrafficFilterIptablesCommandBuilder::GetHookPointName(hook);
        std::string jumpCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteJumpCommand(
            hookName, chain, IptablesName::FILTER);
        NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
            jumpCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
    }
    for (const auto& chain : {info.chainNameIn, info.chainNameOut, info.chainNameFwd}) {
        std::string flushChainCmd = NetTrafficFilterIptablesCommandBuilder::BuildFlushChainCommand(chain,
            IptablesName::FILTER);
        NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
            flushChainCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
        std::string deleteChainCmd = NetTrafficFilterIptablesCommandBuilder::BuildDeleteChainCommand(chain,
            IptablesName::FILTER);
        NetTrafficFilterIptablesCommandBuilder::ExecuteIptablesCommand(
            deleteChainCmd, TrafficFilterIPFamily::IP_FAMILY_V4V6);
    }
}
#endif

int32_t NetTrafficFilterNFQueueCore::DestroyByBundleName(const std::string &bundleName)
{
    std::vector<uint16_t> queueNums;
    OHOS::sptr<NfqCtx> handle = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        handle = GetNFQHandleFromBundleName(bundleName);
        if (handle == nullptr) {
            return TRAFFICFILTER_ERROR_NOT_FOUND;
        }
        for (size_t i = 0; i < NFQ_MAX_QUEUES; i++) {
            if (handle->queues[i] == nullptr) {
                continue;
            }
            queueNums.emplace_back(handle->queues[i]->queueNum);
        }
    }
    for (const auto& queueNum : queueNums) {
        DestroyQueue(queueNum);
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        NetsysController::GetInstance().NfqClose(handle);
    }
    return TRAFFICFILTER_OK;
}

int32_t NetTrafficFilterNFQueueCore::DestroyQueue(uint16_t queueNum)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = queues_.find(queueNum);
    if (it == queues_.end()) {
        return TRAFFICFILTER_ERROR_NOT_FOUND;
    }
    if (it->second.qh != nullptr) {
        NetTrafficFilterPacketRuleManager::GetInstance().ClearPacketRule(it->second.packetControllerId);
        NetsysController::GetInstance().NfqQueueDestroy(it->second.nfqHandle, it->second.qh);
    }
#ifndef NETMANAGER_TEST
    DestroyIptables(it->second);
#endif
    queues_.erase(it);
    return TRAFFICFILTER_OK;
}