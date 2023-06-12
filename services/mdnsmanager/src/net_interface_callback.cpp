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

#include "net_interface_callback.h"

#include <sys/types.h>
#include <unistd.h>
#include "mdns_manager.h"

namespace OHOS {
namespace NetManagerStandard {
NetInterfaceStateCallback::NetInterfaceStateCallback() {}

int32_t NetInterfaceStateCallback::OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName,
                                                             int32_t flags, int32_t scope)
{
    NETMGR_EXT_LOG_D("MDNS_LOG OnInterfaceAddressUpdated, addr:[%{public}s], iface:[%{public}s], scope:[%{public}d]",
                     addr.c_str(), ifName.c_str(), scope);
    MDnsManager::GetInstance().RestartMDnsProtocolImpl();
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName,
                                                             int32_t flags, int32_t scope)
{
    NETMGR_EXT_LOG_D("MDNS_LOG OnInterfaceAddressRemoved, addr:[%{public}s], iface:[%{public}s], scope:[%{public}d]",
                     addr.c_str(), ifName.c_str(), scope);
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceAdded(const std::string &ifName)
{
    NETMGR_EXT_LOG_D("MDNS_LOG OnInterfaceAdded, iface:[%{public}s]", ifName.c_str());
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceRemoved(const std::string &ifName)
{
    NETMGR_EXT_LOG_D("MDNS_LOG OnInterfaceRemoved, iface:[%{public}s]", ifName.c_str());
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceChanged(const std::string &ifName, bool up)
{
    NETMGR_EXT_LOG_D("MDNS_LOG OnInterfaceChanged, iface:[%{public}s] -> Up:[%{public}d]", ifName.c_str(), up);
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
{
    NETMGR_EXT_LOG_I("MDNS_LOG OnInterfaceLinkStateChanged, iface:[%{public}s] -> Up:[%{public}d]", ifName.c_str(), up);
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS