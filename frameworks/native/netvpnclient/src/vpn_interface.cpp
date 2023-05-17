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

#include "vpn_interface.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/ipv6_route.h>
#include <linux/route.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
constexpr std::string_view TUN_CARD_NAME = "vpn-tun";
} // namespace

int32_t VpnInterface::CreateVpnInterface()
{
    return 0;
}

void VpnInterface::DestoryVpnInterface()
{
}

std::string_view VpnInterface::GetName()
{
    return TUN_CARD_NAME;
}

int32_t VpnInterface::SetVpnMtu(int mtu)
{
    return 0;
}

int32_t VpnInterface::SetVpnAddress(const std::string &ipAddr, int prefix)
{
    return 0;
}

int32_t VpnInterface::SetVpnUp()
{
    return 0;
}

int32_t VpnInterface::SetVpnDown()
{
    return 0;
}

} // namespace NetManagerStandard
} // namespace OHOS
