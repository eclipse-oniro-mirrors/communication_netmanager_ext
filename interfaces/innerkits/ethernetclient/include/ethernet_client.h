/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ETHERNET_MANAGER_H
#define ETHERNET_MANAGER_H

#include <string>

#include "parcel.h"
#include "singleton.h"

#include "i_ethernet_service.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetClient {
    DECLARE_DELAYED_SINGLETON(EthernetClient)

public:
    /**
     * @brief Set the network interface configuration
     *
     * @param Network interface name
     * @param Network interface parameters
     * @return Returns 0 as success, other values as failure
     */
    int32_t SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic);
    /**
     * @brief Gets the network interface configuration parameters
     *
     * @param Network interface name
     * @return Parameter is returned on success, null on failure
     */
    sptr<InterfaceConfiguration> GetIfaceConfig(const std::string &iface);
    /**
     * @brief Gets the network interface configuration parameters
     *
     * @param Network interface name
     * @return Returns 1 for device open (active), 0 for device closed (inactive), and -1 for failure
     */
    int32_t IsIfaceActive(const std::string &iface);
    /**
     * @brief Gets the list of active devices
     *
     * @return Return to device List
     */
    std::vector<std::string> GetAllActiveIfaces();
    int32_t ResetFactory();

private:
    class EthernetDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit EthernetDeathRecipient(EthernetClient &client) : client_(client) {}
        ~EthernetDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        EthernetClient &client_;
    };

private:
    sptr<IEthernetService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<IEthernetService> ethernetService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_MANAGER_H