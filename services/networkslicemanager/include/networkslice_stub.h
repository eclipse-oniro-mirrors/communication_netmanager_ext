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

#ifndef NETWORKSLICE_STUB_H
#define NETWORKSLICE_STUB_H

#include <map>

#include "i_networkslice_service.h"
#include "iremote_stub.h"
#include "netmanager_ext_log.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkSliceStub : public IRemoteStub<INetworkSliceService> {
    using NetworkSliceServiceFunc = int32_t (NetworkSliceStub::*)(MessageParcel &, MessageParcel &);

public:
    NetworkSliceStub();
    ~NetworkSliceStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnSetNetworkSlicePolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetworkSliceInitUePolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetworkSliceAllowedNssaiRpt(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetworkSliceEhplmnRpt(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetRouteSelectionDescriptorByDNN(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetRSDByNetCap(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetSaState(MessageParcel &data, MessageParcel &reply);
private:
    std::map<uint32_t, NetworkSliceServiceFunc> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NETWORKSLICE_STUB_H */
