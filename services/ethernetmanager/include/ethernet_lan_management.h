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

#ifndef ETHERNET_LAN_MANAGEMENT_H
#define ETHERNET_LAN_MANAGEMENT_H

#include "dev_interface_state.h"
#include "interface_configuration.h"
#include "net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetLanManagement {
public:
    EthernetLanManagement();
    ~EthernetLanManagement() = default;
    int32_t SetIp(const NetLinkInfo &newNetLinkInfo);
    int32_t DelIp(const NetLinkInfo &newNetLinkInfo);
    int32_t SetRoute(const NetLinkInfo &newNetLinkInfo);
    int32_t DelRoute(const NetLinkInfo &newNetLinkInfo);
    int32_t UpdateLanLinkInfo(sptr<DevInterfaceState> &devState);
    int32_t ReleaseLanNetLink(sptr<DevInterfaceState> &devState);
    void GetOldLinkInfo(sptr<DevInterfaceState> &devState);

private:
    NetLinkInfo netLinkInfo_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_LAN_MANAGEMENT_H
