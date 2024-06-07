/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NET_FIREWALL_STUB_H
#define NET_FIREWALL_STUB_H

#include <map>

#include "i_netfirewall_service.h"
#include "iremote_stub.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallStub : public IRemoteStub<INetFirewallService> {
    using NetFirewallServiceFunc = int32_t (NetFirewallStub::*)(MessageParcel &, MessageParcel &);
    struct ServicePermissionAndFunc {
        std::string strPermission;
        NetFirewallServiceFunc serviceFunc;
    };

public:
    NetFirewallStub();
    
    ~NetFirewallStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnSetNetFirewallPolicy(MessageParcel &data, MessageParcel &reply);

    int32_t OnGetNetFirewallPolicy(MessageParcel &data, MessageParcel &reply);

    int32_t OnAddNetFirewallRule(MessageParcel &data, MessageParcel &reply);

    int32_t OnUpdateNetFirewallRule(MessageParcel &data, MessageParcel &reply);

    int32_t OnDeleteNetFirewallRule(MessageParcel &data, MessageParcel &reply);

    int32_t OnGetNetFirewallRules(MessageParcel &data, MessageParcel &reply);

    int32_t OnGetNetFirewallRule(MessageParcel &data, MessageParcel &reply);

    int32_t OnGetInterceptRecords(MessageParcel &data, MessageParcel &reply);

    int32_t CheckFirewallPermission(std::string &strPermission);

private:
    std::map<uint32_t, ServicePermissionAndFunc> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_FIREWALL_STUB_H */
