/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef NETFIREWALL_INTERCEPT_RECORD_CALLBACK_PROXY_H
#define NETFIREWALL_INTERCEPT_RECORD_CALLBACK_PROXY_H

#include "i_net_intercept_record_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
class NetFirewallInterceptRecordCallbackProxy : public IRemoteProxy<INetInterceptRecordCallback> {
public:
    explicit NetFirewallInterceptRecordCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetFirewallInterceptRecordCallbackProxy() = default;

    int32_t OnInterceptRecord(const sptr<NetManagerStandard::InterceptRecord> &record) override;

private:
    static inline BrokerDelegator<NetFirewallInterceptRecordCallbackProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETFIREWALL_INTERCEPT_RECORD_CALLBACK_PROXY_H