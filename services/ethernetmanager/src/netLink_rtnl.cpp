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

#include "netLink_rtnl.h"

#include <thread>

#include "securec.h"
#include "netmgr_ext_log_wrapper.h"
#include "ethernet_constants.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NetLinkRtnl::seq_ = 0;
NetLinkRtnl::NetLinkRtnl()
{
}

NetLinkRtnl::~NetLinkRtnl() {}

void NetLinkRtnl::NetLinkListenerThead()
{
    int32_t netLinkSocket = 0;
    netLinkSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (netLinkSocket < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl NetLinkListenerThead netLinkSocket create socket failed");
        return;
    }
    if (setsockopt(netLinkSocket, SOL_SOCKET, SO_RCVBUF, &NLK_SOCK_BUF_LEN, sizeof(NLK_SOCK_BUF_LEN)) != 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl NetLinkListenerThead setsockopt failed");
        return;
    }
    struct sockaddr_nl sa;
    bzero(&sa, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_IFADDR | RTMGRP_IPV6_ROUTE;
    if (bind(netLinkSocket, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl NetLinkListenerThead bind failed");
        return;
    }
    fd_set rdSet;
    struct timeval timeout;
    timeout.tv_sec = NLK_TV_SEC;
    timeout.tv_usec = NLK_TV_USEC;
    NETMGR_EXT_LOG_D("NetLinkRtnl NetLinkListenerThead init suc, create netLinkSocket[%{public}d]", netLinkSocket);
    while (true) {
        FD_ZERO(&rdSet);
        FD_SET(netLinkSocket, &rdSet);
        NetLinkHandle(netLinkSocket, rdSet, timeout);
    }
}

void NetLinkRtnl::Init()
{
    NETMGR_EXT_LOG_D("NetLinkRtnl Init thread start");
    std::thread nlk(&NetLinkRtnl::NetLinkListenerThead, this);
    nlk.detach();
    return;
}

void NetLinkRtnl::RegisterHandle(sptr<NlkEventHandle> h)
{
    nlkHandles_.push_back(h);
    NETMGR_EXT_LOG_D("NetLinkRtnl RegisterHandle nlkHandles_ size[%{public}zd]", nlkHandles_.size());
}

void NetLinkRtnl::ProcessIfInfoMsg(const struct nlmsghdr &nh)
{
    NETMGR_EXT_LOG_D("NetLinkRtnl ProcessIfInfoMsg process");
    int32_t len = 0;
    struct rtattr *tb[IFLA_MAX + 1];
    bzero(tb, sizeof(tb));
    struct ifinfomsg *ifInfo = reinterpret_cast<struct ifinfomsg*>(NLMSG_DATA(&nh));
    if (ifInfo->ifi_type != ARPHRD_ETHER) {
        NETMGR_EXT_LOG_E("NetLinkRtnl ProcessIfInfoMsg ifi_type is not ARPHRD_ETHER");
        return;
    }
    len = nh.nlmsg_len - NLMSG_SPACE(sizeof(*ifInfo));
    auto attr = IFLA_RTA(ifInfo);
    for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
        if (attr->rta_type <= IFLA_MAX) {
            tb[attr->rta_type] = attr;
        }
    }
    NlkEventInfo info;
    if (tb[IFLA_IFNAME]) {
        info.iface_ = std::string(reinterpret_cast<char*>(RTA_DATA(tb[IFLA_IFNAME])));
    }
    info.ifiFlags_ = ifInfo->ifi_flags;
    for (auto it = nlkHandles_.begin(); it != nlkHandles_.end(); ++it) {
        (*it)->Handle(info);
    }
}

void NetLinkRtnl::ProcessLinkEventMsg(std::unique_ptr<int8_t> &pBuff, int32_t len)
{
    struct nlmsghdr *nh = nullptr;
    if (!pBuff) {
        return;
    }
    for (nh = reinterpret_cast<struct nlmsghdr*>(pBuff.release()); NLMSG_OK(nh, len);
        nh = NLMSG_NEXT(nh, len)) {
        if (!nh) {
            break;
        }
        NETMGR_EXT_LOG_D("NetLinkRtnl ProcessLinkEventMsg nh nlmsg_type[%{public}d]", nh->nlmsg_type);
        switch (nh->nlmsg_type) {
            case RTM_NEWLINK:
            case RTM_DELLINK:
                ProcessIfInfoMsg(*nh);
                break;
            default:
                break;
        } // switch
    } // for
}

int NetLinkRtnl::NetLinkHandle(int32_t netLinkSocket, fd_set& rdSet, struct timeval& timeout)
{
    int32_t selectFd = select(netLinkSocket + 1, &rdSet, nullptr, nullptr, &timeout);
    if (selectFd < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl NetLinkHandle select failed");
        return ETHERNET_ERROR;
    } else if (selectFd > 0) {
        if (FD_ISSET(netLinkSocket, &rdSet)) {
            int8_t buff[NLK_BUFF_LEN];
            bzero(buff, sizeof(buff));
            int readRet = read(netLinkSocket, buff, sizeof(buff));
            std::unique_ptr<int8_t> pBuff(buff);
            ProcessLinkEventMsg(pBuff, readRet);
        } // if
    } // else if
    return ETHERNET_SUCCESS;
}

std::vector<uint8_t> NetLinkRtnl::GetHWaddr(const std::string &devName)
{
    int32_t sock = 0;
    struct ifreq ifr;
    uint8_t macaddr[NLK_HWADDR_BUF_LEN];
    bzero(macaddr, sizeof(macaddr));
    std::vector<uint8_t> hwAddr;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl GetHWaddr create socket error sock[%{public}d]", sock);
        return hwAddr;
    }
    strcpy_s(ifr.ifr_name, sizeof(ifr.ifr_name), devName.c_str());
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl GetHWaddr ioctl SIOCGIFHWADDR error");
        close(sock);
        return hwAddr;
    }
    memcpy_s(macaddr, NLK_HWADDR_BUF_LEN, ifr.ifr_hwaddr.sa_data, NLK_HWADDR_LEN);
    for (int32_t i = 0; i < NLK_HWADDR_LEN; i++) {
        hwAddr.push_back(macaddr[i]);
    }
    close(sock);
    return hwAddr;
}

void NetLinkRtnl::GetLinkInfo(std::vector<NlkEventInfo> &infos)
{
    struct {
        struct nlmsghdr nlh;
        struct ifinfomsg ifm;
        int8_t buf[NLK_BUFF_LEN];
    } req = {
        .nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
        .nlh.nlmsg_type = RTM_GETLINK,
        .nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST,
        .nlh.nlmsg_seq = ++seq_,
        .ifm.ifi_family = AF_UNSPEC,
    };
    int32_t fd = CreateNetLinkSocket();
    if (fd < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl GetLinkInfo CreateNetLinkSocket failed");
        return;
    }
    int32_t ret = NetLinkSendMsg(fd, req.nlh);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl GetLinkInfo NetLinkSendMsg failed");
        return;
    }
    int8_t buff[NLK_SOCK_BUF_LEN];
    bzero(buff, sizeof(buff));
    ret = read(fd, buff, sizeof(buff));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("read failed");
    }
    std::unique_ptr<int8_t> pBuff(buff);
    ProcessReadMsg(pBuff, ret, infos);
}

void NetLinkRtnl::ProcessReadMsg(std::unique_ptr<int8_t> &pBuff, int32_t len, std::vector<NlkEventInfo> &infos)
{
    struct nlmsghdr *nh = nullptr;
    if (pBuff == nullptr) {
        return;
    }
    for (nh = reinterpret_cast<struct nlmsghdr*>(pBuff.release()); NLMSG_OK(nh, len);
        nh = NLMSG_NEXT(nh, len)) {
        if (!nh) {
            break;
        }
        NETMGR_EXT_LOG_D("NetLinkRtnl ProcessReadMsg nh nlmsg_type[%{public}d]", nh->nlmsg_type);
        switch (nh->nlmsg_type) {
            case RTM_NEWLINK:
            case RTM_DELLINK:
                infos.push_back(ProcessLinkMsg(*nh));
                break;
            default:
                break;
        } // switch
    } // for
}

NlkEventInfo NetLinkRtnl::ProcessLinkMsg(const struct nlmsghdr &nh)
{
    NETMGR_EXT_LOG_D("NetLinkRtnl ProcessIfInfoMsg process");
    int32_t len = 0;
    struct rtattr *tb[IFLA_MAX + 1];
    bzero(tb, sizeof(tb));
    NlkEventInfo info;
    struct ifinfomsg *ifInfo = reinterpret_cast<struct ifinfomsg *>(NLMSG_DATA(&nh));
    if (ifInfo->ifi_type != ARPHRD_ETHER) {
        NETMGR_EXT_LOG_E("NetLinkRtnl ProcessLinkMsg ifi_type is not ARPHRD_ETHER");
        return info;
    }
    len = nh.nlmsg_len - NLMSG_SPACE(sizeof(*ifInfo));
    auto attr = IFLA_RTA(ifInfo);
    for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
        if (attr->rta_type <= IFLA_MAX) {
            tb[attr->rta_type] = attr;
        }
    }
    if (tb[IFLA_IFNAME]) {
        info.iface_ = std::string(reinterpret_cast<char*>(RTA_DATA(tb[IFLA_IFNAME])));
    }
    info.ifiFlags_ = ifInfo->ifi_flags;
    return info;
}

int32_t NetLinkRtnl::CreateNetLinkSocket()
{
    socklen_t addr_len = 0;
    struct sockaddr_nl snl;
    int32_t fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl CreateNetLinkSocket create socket error");
        return ETHERNET_ERROR;
    }
    int32_t ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl CreateNetLinkSocket fcntl error");
        close(fd);
        return ETHERNET_ERROR;
    }
    bzero(&snl, sizeof(snl));
    snl.nl_family = AF_NETLINK;
    snl.nl_groups = 0;
    ret = bind(fd, reinterpret_cast<struct sockaddr*>(&snl), sizeof(snl));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("NetLinkRtnl CreateNetLinkSocket bind error");
        close(fd);
        return ETHERNET_ERROR;
    }
    addr_len = sizeof (snl);
    ret = getsockname(fd, reinterpret_cast<struct sockaddr*>(&snl), &addr_len);
    if (ret < 0 || addr_len != sizeof (snl)) {
        NETMGR_EXT_LOG_E("NetLinkRtnl CreateNetLinkSocket getsockname error");
        close(fd);
        return ETHERNET_ERROR;
    }
    if (snl.nl_family != AF_NETLINK) {
        NETMGR_EXT_LOG_E("NetLinkRtnl CreateNetLinkSocket nl_family error");
        close(fd);
        return ETHERNET_ERROR;
    }
    seq_ = time(NULL);
    return fd;
}

int32_t NetLinkRtnl::NetLinkSendMsg(int32_t fd, struct nlmsghdr &nlh)
{
    struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
    struct iovec iov = {
        .iov_base = &nlh,
        .iov_len = nlh.nlmsg_len
    };
    struct msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };
    return sendmsg(fd, &msg, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS