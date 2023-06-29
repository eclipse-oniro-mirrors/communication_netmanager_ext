/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef I_ETHERNET_SERVICE_H
#define I_ETHERNET_SERVICE_H

#include <string>
#include <vector>

#include "ethernet_ipc_interface_code.h"
#include "interface_configuration.h"
#include "interface_state_callback.h"
#include "interface_type.h"
#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t GET_CFG_SUC = 1;
class IEthernetService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.IEthernetService");

public:
    virtual int32_t SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic) = 0;
    virtual int32_t GetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ifaceConfig) = 0;
    virtual int32_t IsIfaceActive(const std::string &iface, int32_t &activeStatus) = 0;
    virtual int32_t GetAllActiveIfaces(std::vector<std::string> &activeIfaces) = 0;
    virtual int32_t ResetFactory() = 0;
    virtual int32_t RegisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback) = 0;
    virtual int32_t UnregisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback) = 0;
    virtual int32_t SetInterfaceUp(const std::string &iface) = 0;
    virtual int32_t SetInterfaceDown(const std::string &iface) = 0;
    virtual int32_t GetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg) = 0;
    virtual int32_t SetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_ETHERNET_SERVICE_H
