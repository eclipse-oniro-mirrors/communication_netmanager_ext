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

#ifndef WEARABLE_DISTRIBUTED_NET_CLIENT_H
#define WEARABLE_DISTRIBUTED_NET_CLIENT_H

#include <atomic>
#include "iwearable_distributed_net.h"
#include "parcel.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class WearableDistributedNetLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    bool IsFailed();
    const sptr<IRemoteObject> &GetRemoteObject() const;

private:
    std::atomic_bool loadSAFailed_ = false;
    sptr<IRemoteObject> remoteObject_ = nullptr;
    std::mutex loadMutex_;
};

class WearableDistributedNetClient {
public:
    ~WearableDistributedNetClient();
    /**
     * @brief Setup wearable distributed net
     *
     * @param tcpPortId tcp port id
     * @param udpPortId udp port id
     * @param isMetered is metered or not
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetupWearableDistributedNet(const int32_t tcpPortId, const int32_t udpPortId, const bool isMetered);

    /**
     * @brief Enable wearable distributed net
     *
     * @param enableFlag is enable or disable
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t EnableWearableDistributedNet(bool enableFlag);

    /**
     * @brief Teardown wearable distributed net
     *
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t TearDownWearableDistributedNet();

    /**
     * @brief update wearable distributed net metered status
     *
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UpdateWearableDistributedNetMeteredStatus(const bool isMetered);
    static WearableDistributedNetClient &GetInstance()
    {
        static WearableDistributedNetClient ins;
        return ins;
    }
private:
    class WearableDistributedNetDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit WearableDistributedNetDeathRecipient(WearableDistributedNetClient &client) : client_(client) {}
        ~WearableDistributedNetDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        WearableDistributedNetClient &client_;
    };

    sptr<IWearableDistributedNet> GetProxy();
    void RestartWearableDistributedNetManagerSysAbility();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<IWearableDistributedNet> wearableDistributedNetService_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::mutex loadSaMutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // WEARABLE_DISTRIBUTED_NET_CLIENT_H
