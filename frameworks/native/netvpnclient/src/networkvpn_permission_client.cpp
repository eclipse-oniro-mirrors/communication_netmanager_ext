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

#include "networkvpn_permission_client.h"

#include <mutex>

#include "iservice_registry.h"
#include "net_manager_constants.h"
#include "refbase.h"
#include "system_ability_definition.h"
#include "network_vpn_service_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t RequestVpnPermission(int32_t uid, const std::string& bundleName, const std::string& abilityName,
    bool &isAuthorized)
{
    return NetworkVpnPermissionClient::GetInstance().RequestVpnPermission(uid, bundleName, abilityName, isAuthorized);
}

// Pimpl implementation - hide INetworkVpnService dependency in .cpp file
class NetworkVpnPermissionClient::Impl {
public:
    sptr<INetworkVpnService> GetProxy()
    {
        // LCOV_EXCL_START
        std::lock_guard<std::mutex> lock(mutex_);
        if (networkVpnService_ != nullptr) {
            return networkVpnService_;
        }
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            return nullptr;
        }
        sptr<IRemoteObject> remote = sam->GetSystemAbility(COMM_VPN_MANAGER_SYS_ABILITY_ID);
        if (remote == nullptr) {
            return nullptr;
        }
        auto proxy = iface_cast<INetworkVpnService>(remote);
        if (proxy == nullptr) {
            return nullptr;
        }
        // LCOV_EXCL_STOP
        networkVpnService_ = proxy;
        return networkVpnService_;
    }

    std::mutex mutex_;
    sptr<INetworkVpnService> networkVpnService_ = nullptr;
};

NetworkVpnPermissionClient::NetworkVpnPermissionClient()
    : impl_(std::make_unique<Impl>())
{}

NetworkVpnPermissionClient::~NetworkVpnPermissionClient() = default;

NetworkVpnPermissionClient &NetworkVpnPermissionClient::GetInstance()
{
    static NetworkVpnPermissionClient instance;
    return instance;
}

int32_t NetworkVpnPermissionClient::RequestVpnPermission(int32_t uid, const std::string& bundleName,
    const std::string& abilityName, bool &isAuthorized)
{
    if (bundleName.empty()) {
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }

    auto proxy = impl_->GetProxy();
    // LCOV_EXCL_START
    if (proxy == nullptr) {
        return NETMANAGER_EXT_ERR_GET_PROXY_FAIL;
    }
    // LCOV_EXCL_STOP
    return proxy->RequestVpnPermission(uid, bundleName, abilityName, isAuthorized);
}

} // namespace NetManagerStandard
} // namespace OHOS
