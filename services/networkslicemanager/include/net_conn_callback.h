/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef NETWORKSLICE_NET_CONN_CALLBACK_H
#define NETWORKSLICE_NET_CONN_CALLBACK_H
 
#include "net_conn_callback_stub.h"
 
namespace OHOS {
namespace NetManagerStandard {
using NetManagerStandard::NetConnCallbackStub;
using NetManagerStandard::NetHandle;
using NetManagerStandard::NetAllCapabilities;
using NetManagerStandard::NetLinkInfo;
 
class NetConnCallback : public NetConnCallbackStub {
public:
    int32_t NetAvailable(sptr<NetHandle> &netHandle) override;
    int32_t NetLost(sptr<NetHandle> &netHandle) override;
    int32_t NetUnavailable() override;
 
    void SetNetCap(NetCap netCap);
    void CacheRequestUid(int32_t uid);
    void RemoveRequestUid(int32_t uid);
    std::set<int32_t> GetRequestUids();
    void SetUid(int32_t uid);
    int32_t GetUid();
    int32_t GetNetId();
    NetCap GetNetCap();
private:
    NetCap mNetCap_ { NET_CAPABILITY_END };
    std::set<int32_t> mRequestUids_;
    int32_t mUid_;
    int32_t netId_;
};
 
} // namespace NetMgrEnhanced
} // namespace OHOS
#endif
