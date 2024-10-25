/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef WEARABLE_DISTRIBUTED_NET_LINK_INFO_H
#define WEARABLE_DISTRIBUTED_NET_LINK_INFO_H

#include <fstream>
#include "cJSON.h"
#include "inet_addr.h"
#include "net_link_info.h"
#include "net_manager_constants.h"
#include "route.h"
#include "wearable_distributed_net_config_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class WearableDistributedNetLinkInfo {
public:
    bool ReadSystemNetlinkinfoConfiguration();
    void SetInterFaceName(NetLinkInfo &linkInfo);
    int32_t SetNetLinkIPInfo(NetLinkInfo &linkInfo);
    int32_t SetNetLinkRouteInfo(NetLinkInfo &linkInfo);
    int32_t SetDnsLists(NetLinkInfo &linkInfo);
    void SetMtu(NetLinkInfo &linkInfo);
    int32_t SetInterfaceDummyUp();

private:
    std::string GetPrimaryDnsLists();
    std::string GetSecondDnsLists();
    std::string GetIfaceName();
    std::string GetDefaultNetMask();
    std::string GetNetIfaceAddress();
    std::string GetIpv4DeRouteAddr();
    std::string GetDummyAddress();
    std::string GetIpv4AddrNetMask();
    std::string GetRouteEstinationAddr();

    bool ParseDnsLists(const cJSON &json);
    bool ParseIfaceName(const cJSON &json);
    bool ParseDefaultNetMask(const cJSON &json);
    bool ParseNetIfaceAddress(const cJSON &json);
    bool ParseIpv4DeRouteAddr(const cJSON &json);
    bool ParseDummyAddress(const cJSON &json);
    bool ParseIpv4AddrNetMask(const cJSON &json);
    bool ParseRouteDestinationAddr(const cJSON &json);

    bool ReadNetlinkinfoInterfaces(const cJSON &json);
    std::string ReadJsonFile();

private:
    std::string primaryDnsLists_;
    std::string secondDnsLists_;
    std::string ifaceName_;
    std::string defaultNetMask_;
    std::string netIfaceAddress_;
    std::string ipv4DeRouteAddr_;
    std::string dummyAddress_;
    std::string ipv4AddrNetMask_;
    std::string routeDestinationAddr_;
    std::string configPath_ = WEARABLE_DISTRIBUTED_NET_CONFIG_PATH;
};

int32_t CreateNetLinkInfo(NetLinkInfo &linkInfo);
int32_t SetInterfaceDummyDown();
int32_t SetInterfaceDummyUp();
} // namespace NetManagerStandard
} // namespace OHOS
#endif // WEARABLE_DISTRIBUTED_NET_LINK_INFO_H
