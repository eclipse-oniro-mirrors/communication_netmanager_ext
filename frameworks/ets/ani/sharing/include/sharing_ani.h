/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef NET_SHARING_ANI_H
#define NET_SHARING_ANI_H

#include <cstdint>
#include <memory>

#include "cxx.h"
#include "networkshare_client.h"
#include "sharing_event_callback_stub.h"

namespace OHOS {
namespace NetManagerAni {
struct SharingCallback;

inline bool IsSharingSupported(int32_t &ret)
{
    int32_t supported = 0;
    ret = DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->IsSharingSupported(supported);
    return supported == 1;
}

inline bool IsSharing(int32_t &ret)
{
    int32_t sharing = 0;
    ret = DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->IsSharing(sharing);
    return sharing == 1;
}

inline int32_t StartSharing(NetManagerStandard::SharingIfaceType const &type)
{
    return DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->StartSharing(type);
}

inline int32_t StopSharing(NetManagerStandard::SharingIfaceType const &type)
{
    return DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->StopSharing(type);
}

inline int32_t GetStatsRxBytes(int32_t &bytes)
{
    return DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->GetStatsRxBytes(bytes);
}

inline int32_t GetStatsTxBytes(int32_t &bytes)
{
    return DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->GetStatsTxBytes(bytes);
}

inline int32_t GetStatsTotalBytes(int32_t &bytes)
{
    return DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->GetStatsTotalBytes(bytes);
}

inline int32_t GetSharingIfaces(NetManagerStandard::SharingIfaceState const &state, rust::Vec<rust::string> &ifaces)
{
    std::vector<std::string> ifacesVec;
    int32_t ret =
        DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->GetSharingIfaces(state, ifacesVec);
    for (auto iface : ifacesVec) {
        ifaces.push_back(iface);
    }
    return ret;
}

inline int32_t GetSharingState(
    NetManagerStandard::SharingIfaceType const &type, NetManagerStandard::SharingIfaceState &state)
{
    return DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->GetSharingState(type, state);
}

inline int32_t GetSharableRegexs(NetManagerStandard::SharingIfaceType const &type, rust::Vec<rust::String> &ifaceRegexs)
{
    std::vector<std::string> ifaceRegexsVec;
    int32_t ret = DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->GetSharableRegexs(
        type, ifaceRegexsVec);
    for (auto ifaceRegex : ifaceRegexsVec) {
        ifaceRegexs.push_back(ifaceRegex);
    }
    return ret;
}

class NetShareCallbackObserverAni : public NetManagerStandard::SharingEventCallbackStub {
public:
    void OnSharingStateChanged(bool const &isRunning) override;
    void OnInterfaceSharingStateChanged(NetManagerStandard::SharingIfaceType const &type, std::string const &iface,
        NetManagerStandard::SharingIfaceState const &state) override;
    void OnSharingUpstreamChanged(const sptr<NetManagerStandard::NetHandle> netHandle) override;
};

int32_t NetShareObserverRegister();

int32_t NetShareObserverUnRegister();

rust::String GetErrorCodeAndMessage(int32_t &errorCode);

} // namespace NetManagerAni
} // namespace OHOS
#endif // NET_SHARING_ANI_H