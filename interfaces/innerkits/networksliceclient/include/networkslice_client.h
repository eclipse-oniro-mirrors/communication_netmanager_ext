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

#ifndef NETWORKSLICE_CLIENT_H
#define NETWORKSLICE_CLIENT_H

#include <string>

#include "i_networkslice_service.h"
#include "parcel.h"
#include "singleton.h"

namespace OHOS {
namespace NetManagerStandard {

class NetworkSliceClient {
    DECLARE_DELAYED_SINGLETON(NetworkSliceClient)

public:
    int32_t SetNetworkSliceUePolicy(std::vector<uint8_t> buffer);
    int32_t NetworkSliceInitUePolicy();
    int32_t NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer);
    int32_t NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer);
    int32_t GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode);
    int32_t GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas);
    int32_t SetSaState(bool isSaState);
private:
    class NetworkSliceDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetworkSliceDeathRecipient(NetworkSliceClient &client) : client_(client) {}
        ~NetworkSliceDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetworkSliceClient &client_;
    };

private:
    sptr<INetworkSliceService> GetProxy();
    void RecoverCallback();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);
    void DlCloseRemoveDeathRecipient();
private:
    std::mutex mutex_;
    sptr<INetworkSliceService> networksliceService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSLICE_CLIENT_H
