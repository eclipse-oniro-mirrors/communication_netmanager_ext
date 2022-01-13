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

#ifndef ETHERNET_SERVICE_STUB_H
#define ETHERNET_SERVICE_STUB_H

#include <map>
#include "iremote_stub.h"
#include "i_ethernet_service.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetServiceStub : public IRemoteStub<IEthernetService> {
    using EthernetServiceFunc = int32_t (EthernetServiceStub::*)(MessageParcel &, MessageParcel &);

public:
    EthernetServiceStub();
    ~EthernetServiceStub();
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnSetIfaceConfig(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetIfaceConfig(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsIfaceActive(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetAllActiveIfaces(MessageParcel &data, MessageParcel &reply);
    int32_t OnResetFactory(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, EthernetServiceFunc> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_SERVICE_STUB_H

