/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "wearable_distributed_net_agent.h"
#include "wearable_distributed_net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string WEARABLE_DISTRIBUTED_NET_NAME = "wearabledistributednet";

WearableDistributedNetAgent &WearableDistributedNetAgent::GetInstance()
{
    static WearableDistributedNetAgent instance;
    return instance;
}

int32_t WearableDistributedNetAgent::EnableWearableDistributedNetForward(const int32_t tcpPortId,
                                                                         const int32_t udpPortId)
{
    int32_t result = staticConfiguration_.EnableWearableDistributedNetForward(tcpPortId, udpPortId);
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent Forward failed, result:[%{public}d]", result);
    }
    return result;
}

int32_t WearableDistributedNetAgent::DisableWearableDistributedNetForward()
{
    int32_t ret = staticConfiguration_.DisableWearableDistributedNetForward();
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent Disable Forward failed, ret:[%{public}d]", ret);
    }
    return ret;
}

void WearableDistributedNetAgent::ObtainNetCaps(const bool isMetered)
{
    netCaps_ = staticConfiguration_.GetNetCaps(isMetered);
}

void WearableDistributedNetAgent::SetNetSupplierInfo(NetSupplierInfo &networkSupplierInfo)
{
    return staticConfiguration_.GetNetSupplierInfo(networkSupplierInfo);
}

int32_t WearableDistributedNetAgent::SetNetLinkInfo(NetLinkInfo &networkLinkInfo)
{
    return staticConfiguration_.GetNetLinkInfo(networkLinkInfo);
}

int32_t WearableDistributedNetAgent::ClearWearableDistributedNetForwardConfig()
{
    int32_t ret = DisableWearableDistributedNetForward();
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("DisableWearableDistributedNetForward failed, ret:[%{public}d]", ret);
    }
    return ret;
}

int32_t WearableDistributedNetAgent::SetupWearableDistributedNetwork(const int32_t tcpPortId, const int32_t udpPortId,
                                                                     const bool isMetered)
{
    NETMGR_EXT_LOG_I("SetupWearableDistributedNetwork isMetered:[%{public}s]", isMetered ? "true" : "false");
    int32_t result = RegisterNetSupplier(isMetered);
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("WearableDistributedNetAgent RegisterNetSupplier failed, result:[%{public}d]", result);
        return result;
    }

    result = EnableWearableDistributedNetForward(tcpPortId, udpPortId);
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed NetAgent Enable Forward failed, ret:[%{public}d]", result);
        return ClearWearableDistributedNetForwardConfig();
    }
    result = UpdateNetSupplierInfo(true);
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent UpdateNetSupplierInfo failed, result:[%{public}d]", result);
        return ClearWearableDistributedNetForwardConfig();
    }
    result = UpdateNetLinkInfo();
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent UpdateNetLinkInfo failed, result:[%{public}d]", result);
        return ClearWearableDistributedNetForwardConfig();
    }
    result = SetInterfaceDummyUp();
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent SetInterfaceDummyUp failed, result:[%{public}d]", result);
        return ClearWearableDistributedNetForwardConfig();
    }
    return NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNetAgent::TearDownWearableDistributedNetwork()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Agent TearDown Network");
    int32_t result = UnregisterNetSupplier();
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("WearableDistributedNetAgent UnregisterNetSupplier failed, result:[%{public}d]", result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    result = DisableWearableDistributedNetForward();
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent Disable Forward failed, result:[%{public}d]", result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    result = SetInterfaceDummyDown();
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent Set InterfaceDummyDown failed, result:[%{public}d]", result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNetAgent::RegisterNetSupplier(const bool isMetered)
{
    NETMGR_EXT_LOG_I("WearableDistributedNetAgent RegisterNetSupplier");
    if (netSupplierId_ != 0) {
        NETMGR_EXT_LOG_D("NetSupplier [%{public}d] has been registered", netSupplierId_);
        return NETMANAGER_SUCCESS;
    }

    ObtainNetCaps(isMetered);
    return NetConnClient::GetInstance().RegisterNetSupplier(BEARER_BLUETOOTH, WEARABLE_DISTRIBUTED_NET_NAME,
        netCaps_, netSupplierId_);
}

int32_t WearableDistributedNetAgent::UnregisterNetSupplier()
{
    NETMGR_EXT_LOG_I("WearableDistributedNetAgent UnregisterNetSupplier");
    if (netSupplierId_ == 0) {
        NETMGR_EXT_LOG_E("NetSupplier [%{public}d] has been unregistered", netSupplierId_);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    int32_t result = UpdateNetSupplierInfo(false);
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("WearableDistributedNetAgent UnregisterNetSupplier error, result: [%{public}d]", result);
        return result;
    }
    result = NetConnClient::GetInstance().UnregisterNetSupplier(netSupplierId_);
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("WearableDistributedNetAgent UnregisterNetSupplier error, result: [%{public}d]", result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    netSupplierId_  = 0;
    return NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNetAgent::UpdateNetSupplierInfo(const bool isAvailable)
{
    NETMGR_EXT_LOG_I("WearableDistributedNetAgent UpdateNetSupplierInfo");
    if (netSupplierId_ == 0) {
        NETMGR_EXT_LOG_E("WearableDistributedNetAgent UpdateNetSupplierInfo error, netSupplierId is zero");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    SetNetSupplierInfo(netSupplierInfo_);
    auto networkSupplierInfo = sptr<NetSupplierInfo>(&netSupplierInfo_);
    if (networkSupplierInfo == nullptr) {
        NETMGR_EXT_LOG_E("NetSupplierInfo new failed, networkSupplierInfo is nullptr");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    return NetConnClient::GetInstance().UpdateNetSupplierInfo(netSupplierId_, networkSupplierInfo);
}

int32_t WearableDistributedNetAgent::UpdateNetLinkInfo()
{
    NETMGR_EXT_LOG_I("Wearable Distributed Net Agent UpdateNetLinkInfo.");
    if (netSupplierId_ == 0) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent UpdateNetLinkInfo error, netSupplierId_ is zero");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    int32_t result = SetNetLinkInfo(netLinkInfo_);
    if (result != NETMANAGER_SUCCESS) {
        NETMGR_EXT_LOG_E("Wearable Distributed Net Agent GetNetLinkInfo error, result:[%{public}d]", result);
        return result;
    }
    auto networkLinkInfo = sptr<NetLinkInfo>(&netLinkInfo_);
    if (networkLinkInfo == nullptr) {
        NETMGR_EXT_LOG_E("NetLinkInfo new failed, networkLinkInfo is nullptr");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    return NetConnClient::GetInstance().UpdateNetLinkInfo(netSupplierId_, networkLinkInfo);
}
} // namespace NetManagerStandard
}  // namespace OHOS
