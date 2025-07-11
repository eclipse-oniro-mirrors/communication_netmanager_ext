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

#ifndef NET_VPN_INTERFACE_H
#define NET_VPN_INTERFACE_H

#include <atomic>
#include <cstdint>
#include <sys/socket.h>
#include <sys/types.h>

namespace OHOS {
namespace NetManagerStandard {
class VpnInterface {
public:
    VpnInterface() = default;
    ~VpnInterface() = default;

public:
    int32_t GetVpnInterfaceFd();
    void CloseVpnInterfaceFd();
#ifdef SUPPORT_SYSVPN
    void SetSupportMultiVpn(bool isMultiTunVpn);
#endif // SUPPORT_SYSVPN
private:
    int32_t ConnectControl(int32_t sockfd, int32_t nsec);
    int32_t RecvMsgFromUnixServer(int32_t sockfd);

private:
    std::atomic_int tunFd_{0};
#ifdef SUPPORT_SYSVPN
    bool isSupportMultiVpn_ = false;
#endif // SUPPORT_SYSVPN
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_INTERFACE_H
