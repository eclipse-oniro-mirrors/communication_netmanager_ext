/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef I_NETWORKSLICE_SERVICE_H
#define I_NETWORKSLICE_SERVICE_H

#include <map>
#include "iremote_broker.h"
#include "networkslice_ipc_interface_code.h"


namespace OHOS {
namespace NetManagerStandard {
class INetworkSliceService : public IRemoteBroker {
public:
DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetworksliceService");
    virtual int32_t SetNetworkSliceUePolicy(std::vector<uint8_t> buffer) = 0;
    virtual int32_t NetworkSliceInitUePolicy() = 0;
    virtual int32_t NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer) = 0;
    virtual int32_t NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer) = 0;
    virtual int32_t GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode) = 0;
    virtual int32_t GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas) = 0;
    virtual int32_t SetSaState(bool isSaState) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NETWORKSLICE_SERVICE_H
