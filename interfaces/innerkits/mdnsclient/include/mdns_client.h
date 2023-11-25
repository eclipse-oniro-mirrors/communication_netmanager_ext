/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef MDNS_CLIENT_H
#define MDNS_CLIENT_H

#include <map>
#include <string>

#include "parcel.h"
#include "singleton.h"
#include "system_ability_load_callback_stub.h"

#include "i_mdns_event.h"
#include "i_mdns_service.h"
#include "mdns_service_info.h"

namespace OHOS {
namespace NetManagerStandard {
class OnDemandLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    const sptr<IRemoteObject> &GetRemoteObject() const;

private:
    sptr<IRemoteObject> remoteObject_ = nullptr;
};

class MDnsClient {
    DECLARE_DELAYED_SINGLETON(MDnsClient);

public:
/**
     * Register mDNS service instance
     *
     * @param serviceInfo.name Service instance name
     * @param serviceInfo.type Service instance type
     * @param serviceInfo.port Service instance port
     * @param cb callback object
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb);

    /**
     * UnRegister mDNS service instance
     *
     * @param cb callback object used in RegisterService
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UnRegisterService(const sptr<IRegistrationCallback> &cb);

    /**
     * Browse mDNS service instance by service type
     *
     * @param serviceType Service instance type
     * @param cb callback object
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t StartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb);

    /**
     * Stop browse mDNS service instance by service type
     *
     * @param cb callback object used in StartDiscoverService
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t StopDiscoverService(const sptr<IDiscoveryCallback> &cb);

    /**
     * Resolve browse mDNS service instance by service type and name
     *
     * @param serviceInfo.name Service instance name
     * @param serviceInfo.type Service instance type
     * @param serviceInfo.port Service instance port
     * @param cb callback object
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t ResolveService(const MDnsServiceInfo &serviceInfo, const sptr<IResolveCallback> &cb);

    void RestartResume();

private:
    class MDnsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit MDnsDeathRecipient(MDnsClient &client) : client_(client) {}
        ~MDnsDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        MDnsClient &client_;
    };

    sptr<IRemoteObject> LoadSaOnDemand();
    sptr<IMDnsService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

    std::mutex mutex_;
    sptr<IMDnsService> mdnsService_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    sptr<OnDemandLoadCallback> loadCallback_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MDNS_CLIENT_H
