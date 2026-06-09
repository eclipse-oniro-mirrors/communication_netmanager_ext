/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NET_ETHERNET_ANI_H
#define NET_ETHERNET_ANI_H

#include <cstdint>
#include <string>
#include <vector>

#include "cxx.h"
#include "interface_state_callback_stub.h"

namespace OHOS {
namespace NetManagerAni {

bool IsEthernetEnabled(int32_t &ret);
int32_t DisableEthernetInterface();
int32_t EnableEthernetInterface();
int32_t g_getAllActiveIfaces(rust::Vec<rust::String> &activeIfaces);
int32_t IsIfaceActive(const rust::String &iface, int32_t &activeStatus);
rust::String GetIfaceConfig(const rust::String &iface, int32_t &ret);
int32_t SetIfaceConfig(const rust::String &iface, const rust::String &configStr);
rust::String GetDeviceInformation(int32_t &ret);
rust::String GetMacAddress(int32_t &ret);
int32_t RegisterInterfaceStateCallback();
int32_t UnregisterInterfaceStateCallback();
rust::String GetErrorCodeAndMessage(int32_t &errorCode);

class InterfaceStateCallbackObserverAni : public NetManagerStandard::InterfaceStateCallbackStub {
public:
    int32_t OnInterfaceAdded(const std::string &ifName) override;
    int32_t OnInterfaceRemoved(const std::string &ifName) override;
    int32_t OnInterfaceChanged(const std::string &ifName, bool up) override;
};

} // namespace NetManagerAni
} // namespace OHOS

#endif // NET_ETHERNET_ANI_H
