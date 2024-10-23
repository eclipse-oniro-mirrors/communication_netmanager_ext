/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "wearable_distributed_net_static_configuration.h"

namespace OHOS {
namespace NetManagerStandard {

WearableDistributedNetStaticConfiguration::WearableDistributedNetStaticConfiguration()
{
    netCaps_.insert(NET_CAPABILITY_INTERNET);
    netCaps_.insert(NET_CAPABILITY_NOT_VPN);
}

WearableDistributedNetStaticConfiguration::~WearableDistributedNetStaticConfiguration() = default;

int32_t WearableDistributedNetStaticConfiguration::SetNetLinkInfo(NetLinkInfo &linkInfo)
{
    linkInfo.Initialize();
    int32_t result = CreateNetLinkInfo(linkInfo);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("SetNetLinkInfo SetInterfaceDummy failed, result: [%{public}d]", result);
        return result;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t WearableDistributedNetStaticConfiguration::GetNetLinkInfo(NetLinkInfo &linkInfo)
{
    int32_t result = SetNetLinkInfo(linkInfo);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("GetNetLinkInfo SetNetLinkInfo failed, result: [%{public}d]", result);
        return result;
    }
    return NETMANAGER_EXT_SUCCESS;
}

void WearableDistributedNetStaticConfiguration::GetNetSupplierInfo(NetSupplierInfo &supplierInfo)
{
    SetNetSupplierInfo(supplierInfo);
}

int32_t WearableDistributedNetStaticConfiguration::EnableWearableDistributedNetForward(const int32_t tcpPortId,
                                                                                       const int32_t udpPortId)
{
    int32_t ret = wearableDistributedNetForward_.EnableWearableDistributedNetForward(tcpPortId, udpPortId);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Static Configuration Enable Forward failed, ret: [%{public}d]", ret);
        return ret;
    }
    return NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNetStaticConfiguration::DisableWearableDistributedNetForward()
{
    int32_t ret = wearableDistributedNetForward_.DisableWearableDistributedNetForward();
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("WearableDistributedNetStaticConfiguration Disable Forward failed, ret: [%{public}d]", ret);
        return ret;
    }
    return NETMANAGER_SUCCESS;
}

void WearableDistributedNetStaticConfiguration::SetNetMetered(const bool isMetered)
{
    auto it = netCaps_.find(NET_CAPABILITY_NOT_METERED);
    if (isMetered && it != netCaps_.end()) {
        netCaps_.erase(it);
    }
    if (!isMetered && it == netCaps_.end()) {
        netCaps_.insert(NET_CAPABILITY_NOT_METERED);
    }
}

std::set<NetCap> WearableDistributedNetStaticConfiguration::GetNetCaps(const bool isMetered)
{
    SetNetMetered(isMetered);
    return netCaps_;
}
} // namespace NetManagerStandard
} // namespace OHOS
