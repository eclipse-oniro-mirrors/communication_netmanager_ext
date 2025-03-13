/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NETWORKSLICE_SERVICE_H
#define NETWORKSLICE_SERVICE_H

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "refbase.h"
#include "singleton.h"
#include "system_ability.h"
#include "datashare_helper.h"
#include "networkslice_stub.h"
#include "networkslicecommconfig.h"

namespace OHOS {
namespace NetManagerStandard {

class NetworkSliceService : public SystemAbility,
    public NetworkSliceStub,
    public std::enable_shared_from_this<NetworkSliceService> {
    DECLARE_DELAYED_SINGLETON(NetworkSliceService)
    DECLARE_SYSTEM_ABILITY(NetworkSliceService)
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };
    
public:
    static NetworkSliceService &GetInstance();
    int32_t SetNetworkSliceUePolicy(std::vector<uint8_t> buffer) override;
    int32_t NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer) override;
    int32_t NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer) override;
    int32_t GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode) override;
    int32_t GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas) override;
    int32_t SetSaState(bool isSaState) override;

    int32_t NetworkSliceGetRSDByAppDescriptor(std::shared_ptr<GetSlicePara>& getSlicePara);
    int32_t RecvKernelData(void* rcvMsg, int32_t dataLen);
    int GetBundleNameForUid(int32_t uid, std::string &bundleName);
    int32_t BindToNetwork(std::map<std::string, std::string> buffer);
    int32_t DelBindToNetwork(std::map<std::string, std::string> buffer);
    int WriteNetworkSliceApnToDb(std::shared_ptr<DataShare::DataShareHelper> networksliceApnHelper,
        std::string apntype);
    bool UpdateNetworkSliceApn();
    int32_t NetworkSliceInitUePolicy() override;
    int32_t GetUidByBundleName(const std::string& bundleName);
    std::set<int32_t> GetUidsByBundleName(const std::string& bundleName);
protected:
    void OnStart() override;
    void OnStop() override;

private:
    bool Init();
    void InitModule();
    bool isRegistered_;
    ServiceRunningState state_;
    static NetworkSliceService instance_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSLICE_SERVICE_H
