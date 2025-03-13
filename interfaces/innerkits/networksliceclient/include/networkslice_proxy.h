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

#ifndef NETWORKSLICE_PROXY_H
#define NETWORKSLICE_PROXY_H

#include "i_networkslice_service.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkSliceProxy : public IRemoteProxy<INetworkSliceService> {
public:
    explicit NetworkSliceProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetworkSliceProxy();
    int32_t SetNetworkSliceUePolicy(std::vector<uint8_t> buffer) override;
    int32_t NetworkSliceInitUePolicy() override;
    int32_t NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer) override;
    int32_t NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer) override;
    int32_t GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode) override;
    int32_t GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas) override;
    int32_t SetSaState(bool isSaState) override;
private:
    static inline BrokerDelegator<NetworkSliceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NETWORKSLICE_PROXY_H */
