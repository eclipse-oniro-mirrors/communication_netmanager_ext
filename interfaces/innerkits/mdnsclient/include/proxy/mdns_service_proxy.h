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

#ifndef MDNS_SERVICE_PROXY_H
#define MDNS_SERVICE_PROXY_H

#include "iremote_proxy.h"

#include "i_mdns_service.h"

namespace OHOS {
namespace NetManagerStandard {
class MDnsServiceProxy : public IRemoteProxy<IMDnsService> {
public:
    explicit MDnsServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~MDnsServiceProxy();

    int32_t RegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb);
    int32_t UnRegisterService(const sptr<IRegistrationCallback> &cb);

    int32_t StartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb);
    int32_t StopDiscoverService(const sptr<IDiscoveryCallback> &cb);

    int32_t ResolveService(const MDnsServiceInfo &serviceInfo, const sptr<IResolveCallback> &cb);

private:
    template <class T>
    int32_t CheckParamVaildRemote(const T &cb, MessageParcel &data,
                                                    const sptr<IRemoteObject> &remote)
    {
        if (!data.WriteInterfaceToken(MDnsServiceProxy::GetDescriptor())) {
            NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
            return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
        }

        if (!data.WriteRemoteObject(cb->AsObject().GetRefPtr())) {
            NETMGR_EXT_LOG_E("proxy write callback failed");
            return NETMANAGER_EXT_ERR_WRITE_DATA_FAIL;
        }

        if (remote == nullptr) {
            NETMGR_EXT_LOG_E("Remote is null");
            return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
        }

        return ERR_NONE;
    }
    static inline BrokerDelegator<MDnsServiceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // MDNS_SERVICE_PROXY_H
