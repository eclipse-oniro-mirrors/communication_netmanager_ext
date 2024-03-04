/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef NETWORKVPN_SERVICE_STUB_H
#define NETWORKVPN_SERVICE_STUB_H

#include <map>

#include "i_networkvpn_service.h"
#include "iremote_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkVpnServiceStub : public IRemoteStub<INetworkVpnService> {
    using NetworkVpnServiceFunc = int32_t (NetworkVpnServiceStub::*)(MessageParcel &, MessageParcel &);

    struct ServicePermissionAndFunc {
        std::string strPermission;
        NetworkVpnServiceFunc serviceFunc;
    };

public:
    NetworkVpnServiceStub();
    ~NetworkVpnServiceStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t ReplyPrepare(MessageParcel &data, MessageParcel &reply);
    int32_t ReplySetUpVpn(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyProtect(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyDestroyVpn(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyRegisterVpnEvent(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyUnregisterVpnEvent(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyCreateVpnConnection(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyFactoryResetVpn(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyRegisterBundleName(MessageParcel &data, MessageParcel &reply);

    int32_t CheckVpnPermission(std::string &strPermission);

private:
    std::map<INetworkVpnService::MessageCode, ServicePermissionAndFunc> permissionAndFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKVPN_SERVICE_STUB_H
