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

#ifndef NETWORKVPN_PERMISSION_CLIENT_H
#define NETWORKVPN_PERMISSION_CLIENT_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace NetManagerStandard {
extern "C" int32_t RequestVpnPermission(int32_t uid, const std::string& bundleName, const std::string& abilityName,
    bool &isAuthorized);
/**
 * @brief NetworkVpnPermissionClient permission management class
 *
 * This class provides VPN permission request functionality.
 * Implementation details are hidden using pimpl idiom to avoid exposing
 * INetworkVpnService dependencies in the header file.
 */
class NetworkVpnPermissionClient {
private:
    // Pimpl idiom - hide implementation details
    class Impl;

public:
    NetworkVpnPermissionClient();
    ~NetworkVpnPermissionClient();
    NetworkVpnPermissionClient(const NetworkVpnPermissionClient &) = delete;
    NetworkVpnPermissionClient &operator=(const NetworkVpnPermissionClient &) = delete;

public:
    static NetworkVpnPermissionClient &GetInstance();

    /**
     * request vpn permission
     *
     * @param uid the uid of the application
     * @param bundleName the bundle name of the application
     * @param abilityName the ability name of the application
     * @param isAuthorized whether the vpn permission is authorized (out param)
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t RequestVpnPermission(int32_t uid, const std::string& bundleName, const std::string& abilityName,
        bool &isAuthorized);

private:
    std::unique_ptr<Impl> impl_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKVPN_PERMISSION_CLIENT_H
