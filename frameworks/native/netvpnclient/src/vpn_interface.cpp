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

#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
static const sockaddr_un SERVER_PATH = {AF_UNIX, "/dev/unix/socket/tunfd"};
constexpr int32_t CONNECT_TIMEOUT = 1;
constexpr int32_t INVALID_FD = -1;
} // namespace

int32_t VpnInterface::ConnectControl(int32_t sockfd, int32_t nsec)
{
    uint32_t flags = static_cast<uint32_t>(fcntl(sockfd, F_GETFL, 0));
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    /* EINPROGRESS - Indicates that the connection establishment has been started but is not complete */
    int32_t ret = connect(sockfd, reinterpret_cast<const sockaddr *>(&SERVER_PATH), sizeof(SERVER_PATH));
    if ((ret < 0) && (errno != EINPROGRESS)) {
        NETMGR_EXT_LOG_E("connect error: %{public}d", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    } else if (ret == 0) {
        /* connect completed immediately, This can happen when the server is on the client's host*/
        fcntl(sockfd, F_SETFL, flags); /* restore file status flags */
        NETMGR_EXT_LOG_I("connect success.");
        return NETMANAGER_EXT_SUCCESS;
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    fd_set wset = rset;

    timeval tval;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;
    ret = select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL);
    if (ret < 0) { // select error.
        NETMGR_EXT_LOG_E("select error: %{public}d", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    } else if (ret == 0) { // timeout
        NETMGR_EXT_LOG_E("connect timeout.");
        return NETMANAGER_EXT_ERR_INTERNAL;
    } else { // fd ready
        int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
        socklen_t len = sizeof(result);
        if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &result, &len) < 0) {
                NETMGR_EXT_LOG_E("getsockopt error: %{public}d", errno);
                return NETMANAGER_EXT_ERR_INTERNAL;
            }
        } else {
            NETMGR_EXT_LOG_E("select error: sockfd not set");
            return NETMANAGER_EXT_ERR_INTERNAL;
        }

        if (result != NETMANAGER_EXT_SUCCESS) { // connect failed.
            NETMGR_EXT_LOG_E("connect failed. error: %{public}d", result);
            return NETMANAGER_EXT_ERR_INTERNAL;
        } else {                           // connect success.
            fcntl(sockfd, F_SETFL, flags); /* restore file status flags */
            NETMGR_EXT_LOG_I("connect success.");
            return NETMANAGER_EXT_SUCCESS;
        }
    }
}

int32_t VpnInterface::RecvMsgFromUnixServer(int32_t sockfd)
{
    char buf[1] = {0};
    iovec iov = {
        .iov_base = buf,
        .iov_len = sizeof(buf),
    };
    union {
        cmsghdr align;
        char cmsg[CMSG_SPACE(sizeof(int32_t))];
    } cmsgu;
    if (memset_s(cmsgu.cmsg, sizeof(cmsgu.cmsg), 0, sizeof(cmsgu.cmsg)) != EOK) {
        NETMGR_EXT_LOG_E("memset_s cmsgu.cmsg failed!");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    msghdr message;
    if (memset_s(&message, sizeof(message), 0, sizeof(message)) != EOK) {
        NETMGR_EXT_LOG_E("memset_s message failed!");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    message.msg_iov = &iov;
    message.msg_iovlen = 1;
    message.msg_control = cmsgu.cmsg;
    message.msg_controllen = sizeof(cmsgu.cmsg);
    if (recvmsg(sockfd, &message, 0) < 0) {
        NETMGR_EXT_LOG_E("recvmsg msg error: %{public}d", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    cmsghdr *cmsgh = CMSG_FIRSTHDR(&message);
    if (cmsgh == nullptr) {
        NETMGR_EXT_LOG_E("cmsgh is nullptr");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (cmsgh->cmsg_level != SOL_SOCKET || cmsgh->cmsg_type != SCM_RIGHTS ||
        cmsgh->cmsg_len != CMSG_LEN(sizeof(int32_t))) {
        NETMGR_EXT_LOG_E("cmsg_level: [%{public}d], cmsg_type: [%{public}d], cmsg_len: [%{public}d]", cmsgh->cmsg_level,
                         cmsgh->cmsg_type, cmsgh->cmsg_len);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    if (memcpy_s(&tunFd_, sizeof(tunFd_), CMSG_DATA(cmsgh), sizeof(tunFd_)) != EOK) {
        NETMGR_EXT_LOG_E("memcpy_s cmsgu failed!");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnInterface::GetVpnInterfaceFd()
{
    CloseVpnInterfaceFd();
    int32_t sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        NETMGR_EXT_LOG_E("create unix SOCK_STREAM socket error: %{public}d", errno);
        return INVALID_FD;
    }

    if (ConnectControl(sockfd, CONNECT_TIMEOUT) != NETMANAGER_EXT_SUCCESS) {
        close(sockfd);
        NETMGR_EXT_LOG_E("connect error: %{public}d", errno);
        return INVALID_FD;
    }

    if (RecvMsgFromUnixServer(sockfd) != NETMANAGER_EXT_SUCCESS) {
        close(sockfd);
        return INVALID_FD;
    }

    close(sockfd);
    NETMGR_EXT_LOG_I("receive tun device fd: [%{public}d]", tunFd_.load());
    return tunFd_;
}

void VpnInterface::CloseVpnInterfaceFd()
{
    if (tunFd_ > 0) {
        NETMGR_EXT_LOG_I("close tunfd[%{public}d] of vpn interface", tunFd_.load());
        close(tunFd_);
        tunFd_ = 0;
    }
}

} // namespace NetManagerStandard
} // namespace OHOS
