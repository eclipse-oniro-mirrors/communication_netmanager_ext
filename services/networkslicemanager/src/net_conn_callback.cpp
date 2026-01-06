/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "net_conn_callback.h"
#include "net_manager_constants.h"
#include "networkslicecommconfig.h"
#include "networksliceutil.h"
#include "hwnetworkslicemanager.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NetConnCallback::NetAvailable(sptr<NetHandle> &netHandle)
{
    // LCOV_EXCL_START
    if (netHandle == nullptr) {
        return -1;
    }
    // LCOV_EXCL_STOP
    int32_t netId = netHandle->GetNetId();
    netId_ = netId;
    NETMGR_EXT_LOG_I("netconn callback, netAvailable, netId: %{public}d", netId);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkAvailable(mNetCap_, netId);
    std::shared_ptr<NetworkSliceInfo> info =
        DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetNetworkSliceInfoByParaNetCap(mNetCap_);
    if (info != nullptr) {
        info->setNetId(netId);
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetConnCallback::NetLost(sptr<NetHandle> &netHandle)
{
    // LCOV_EXCL_START
    if (netHandle == nullptr) {
        return -1;
    }
    // LCOV_EXCL_STOP
    int32_t netId = netHandle->GetNetId();
    netId_ = 0;
    NETMGR_EXT_LOG_I("netconn callback, NetLost, netId: %{public}d", netId);
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkLost(mNetCap_, netId);
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetConnCallback::NetUnavailable()
{
    NETMGR_EXT_LOG_E("netconn callback, NetUnavailable");
    DelayedSingleton<HwNetworkSliceManager>::GetInstance()->OnNetworkUnavailable(mNetCap_);
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

void NetConnCallback::SetNetCap(NetCap netCap)
{
    mNetCap_ = netCap;
}

NetCap NetConnCallback::GetNetCap()
{
    return mNetCap_;
}

void NetConnCallback::CacheRequestUid(int32_t uid)
{
    mRequestUids_.insert(uid);
}

void NetConnCallback::RemoveRequestUid(int32_t uid)
{
    mRequestUids_.erase(uid);
}

std::set<int32_t> NetConnCallback::GetRequestUids()
{
    return mRequestUids_;
}

void NetConnCallback::SetUid(int32_t uid)
{
    mUid_ = uid;
}

int32_t NetConnCallback::GetUid()
{
    return mUid_;
}

int32_t NetConnCallback::GetNetId()
{
    return netId_;
}
}
}
