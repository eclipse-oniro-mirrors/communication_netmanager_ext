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

#ifndef NETLINK_RTNL_H
#define NETLINK_RTNL_H

#include <list>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if_arp.h>
#include <linux/route.h>
#include <linux/rtnetlink.h>

#include "nlk_event_handle.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t NLK_SOCK_BUF_LEN = 20480;
constexpr int32_t NLK_BUFF_LEN = 2048;
constexpr int32_t NLK_HWADDR_BUF_LEN = 16;
constexpr int32_t NLK_HWADDR_LEN = 6;
constexpr int32_t NLK_TV_SEC = 5;
constexpr int32_t NLK_TV_USEC = 0;
class NetLinkRtnl {
public:
    NetLinkRtnl();
    ~NetLinkRtnl();
    void NetLinkListenerThead();
    void Init();
    void RegisterHandle(sptr<NlkEventHandle> h);
    static std::vector<uint8_t> GetHWaddr(const std::string &devName);
    static void GetLinkInfo(std::vector<NlkEventInfo> &infos);

private:
    static int32_t CreateNetLinkSocket();
    static int32_t NetLinkSendMsg(int32_t fd, struct nlmsghdr &nlh);
    static void ProcessReadMsg(std::unique_ptr<int8_t> &pBuff, int32_t len, std::vector<NlkEventInfo> &infos);
    static NlkEventInfo ProcessLinkMsg(const struct nlmsghdr &nh);
    int NetLinkHandle(int32_t netLinkSocket, fd_set& rdSet, struct timeval& timeout);
    void ProcessIfInfoMsg(const struct nlmsghdr &nh);
    void ProcessLinkEventMsg(std::unique_ptr<int8_t> &pBuff, int32_t len);

private:
    std::list<sptr<NlkEventHandle>> nlkHandles_;
    static int32_t seq_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETLINK_RTNL_H