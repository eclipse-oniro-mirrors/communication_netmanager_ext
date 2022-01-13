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

#ifndef ETHERNET_SERVICE_PROXY_H
#define ETHERNET_SERVICE_PROXY_H

#include <string>

#include "iremote_proxy.h"
#include "i_ethernet_service.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetServiceProxy : public IRemoteProxy<IEthernetService> {
public:
    explicit EthernetServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~EthernetServiceProxy();
    bool WriteInterfaceToken(MessageParcel &data);

    int32_t SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic) override;
    sptr<InterfaceConfiguration> GetIfaceConfig(const std::string &iface) override;
    int32_t IsIfaceActive(const std::string &iface) override;
    std::vector<std::string> GetAllActiveIfaces() override;
    int32_t ResetFactory() override;

private:
    static inline BrokerDelegator<EthernetServiceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_SERVICE_PROXY_H