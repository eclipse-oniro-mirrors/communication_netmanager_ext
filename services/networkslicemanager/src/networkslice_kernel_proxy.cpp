/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstddef>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "c/executor_task.h"
#include "securec.h"
#include "networkslice_submodule.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkslice_service.h"
#include "networkslice_kernel_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t NETLINK_BUFFER_MAX_SIZE = 4080;
constexpr int32_t NETLINK_NETMANAGEREXT = 49;
constexpr int32_t NETWORKSLICE_REG_MSG = 0;
constexpr int32_t NETWORKSLICE_DATA_MSG = 1;
constexpr int32_t DEFAULT_SLEEP_TIME_US = 100000;
}

NetworkSliceKernelProxy::NetworkSliceKernelProxy()
{
    NETMGR_EXT_LOG_I("NetworkSliceKernelProxy()");
}

NetworkSliceKernelProxy::~NetworkSliceKernelProxy()
{
    NETMGR_EXT_LOG_I("~NetworkSliceKernelProxy()");
    StopRecvThread();
}

void NetworkSliceKernelProxy::RegistHandler(NetworkSliceSubModule moduleId, NetworkSliceServiceBase* handler,
    const std::vector<int16_t>& msgTypeVec)
{
    if (handler == nullptr || msgTypeVec.empty()) {
        NETMGR_EXT_LOG_E("invalid para");
        return;
    }

    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    if (moduleIdHandlerMap_.find(moduleId) == moduleIdHandlerMap_.end()) {
        moduleIdHandlerMap_.insert(std::make_pair(moduleId, handler));
    }

    moduleIdMsgTypesMap_[moduleId].assign(msgTypeVec.begin(), msgTypeVec.end());
    for (auto msgType : msgTypeVec) {
        msgTypeModuleIdsMap_[msgType].insert(moduleId);
    }
}

void NetworkSliceKernelProxy::UnRegistHandler(NetworkSliceSubModule moduleId)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    auto iter = moduleIdHandlerMap_.find(moduleId);
    if (iter != moduleIdHandlerMap_.end()) {
        moduleIdHandlerMap_.erase(iter);
    }

    for (auto msgType : moduleIdMsgTypesMap_[moduleId]) {
        msgTypeModuleIdsMap_[msgType].erase(moduleId);
    }

    moduleIdMsgTypesMap_.erase(moduleId);
}

int32_t NetworkSliceKernelProxy::StartNetlink()
{
    NETMGR_EXT_LOG_I("NetworkSlice StartNetlink start");
    int32_t ret = -1;
    if (netlinkSocket_ < 0) {
        netlinkSocket_ = NetlinkInit();
        if (netlinkSocket_ < 0) {
            NETMGR_EXT_LOG_E("NetworkSlice StartNetlink: socket open failed");
            return ret;
        }
        NETMGR_EXT_LOG_I("NetworkSlice socket connect:%{public}d", netlinkSocket_);
    }
    NetlinkInfo nlreq = {};
    ret = SendMsgToKernel(NETWORKSLICE_REG_MSG, reinterpret_cast<nlmsghdr*>(&nlreq), NLMSG_LENGTH(0));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("NetworkSlice StartNetlink: send msg failed, ret:%{public}d", ret);
    }
    NETMGR_EXT_LOG_I("NetworkSlice StartNetlink success");
    return ret;
}

int32_t NetworkSliceKernelProxy::NetlinkInit()
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    NETMGR_EXT_LOG_I("NetworkSlice NetlinkInit start");
    int32_t fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_NETMANAGEREXT);
    NETMGR_EXT_LOG_I("NetworkSlice NetlinkInit fd = %{public}d", fd);
    if (fd < 0) {
        NETMGR_EXT_LOG_E("NetlinkInitCant:create netlink socket,error:%{public}s\n", strerror(errno));
        return -1;
    }
    int32_t ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        NETMGR_EXT_LOG_E("NetlinkInit:Error on fcntl:%{public}s", strerror(errno));
        close(fd);
        return -1;
    }
    sockaddr_nl ntlAddr = {};
    ntlAddr.nl_family = AF_NETLINK;
    ntlAddr.nl_pid = static_cast<uint32_t>(getprocpid());
    NETMGR_EXT_LOG_I("Netmanager_ext PID:%{public}d", ntlAddr.nl_pid);
    ntlAddr.nl_groups = 0;
    if (bind(fd, reinterpret_cast<sockaddr*>(&ntlAddr), sizeof(ntlAddr)) != 0) {
        NETMGR_EXT_LOG_E("NetlinkInit:Cant bind netlink socket\n");
        close(fd);
        return -1;
    }
    NETMGR_EXT_LOG_I("Netmanager_ext_NetlinkInit success");
    return fd;
}

int32_t NetworkSliceKernelProxy::SendMsgToKernel(int32_t type, nlmsghdr *nlmsg, size_t nlmsgLen)
{
    NETMGR_EXT_LOG_I("NetworkSliceKernelProxy::SendMsgToKernel");
    int32_t ret = -1;
    if (!nlmsg) {
        NETMGR_EXT_LOG_E("SendMsgToKernel:null nlmsg");
        return ret;
    }
    if (netlinkSocket_ < 0) {
        NETMGR_EXT_LOG_E("SendMsgToKernel:socket not open");
        return ret;
    }
    if (nlmsgLen < sizeof(nlmsghdr)) {
        NETMGR_EXT_LOG_E("SendMsgToKernel:nlmsgLen too short");
        return ret;
    }
    nlmsg->nlmsg_len = nlmsgLen;
    nlmsg->nlmsg_flags = 0;
    nlmsg->nlmsg_type = type;
    nlmsg->nlmsg_pid = static_cast<uint32_t>(getprocpid());
    sockaddr_nl ntlAddr = {};
    ntlAddr.nl_family = AF_NETLINK;
    ntlAddr.nl_pid = 0;
    ntlAddr.nl_groups = 0;
    ret = sendto(netlinkSocket_, nlmsg, nlmsgLen, 0, reinterpret_cast<sockaddr*>(&ntlAddr),
        sizeof(sockaddr_nl));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("SendMsgToKernel: (%{public}d) failed", type);
    }
    return ret;
}

int32_t NetworkSliceKernelProxy::SendDataToKernel(KernelMsg &msgData)
{
    NETMGR_EXT_LOG_I("SendDataToKernel");
    int32_t ret = -1;
    if (netlinkSocket_ < 0) {
        netlinkSocket_ = NetlinkInit();
        if (netlinkSocket_ < 0) {
            NETMGR_EXT_LOG_E("SendDataToKernel:socket open fail");
            return ret;
        }
        NETMGR_EXT_LOG_I("socket change:%{public}d", netlinkSocket_);
    }

    int16_t length = msgData.len;
    if (length > NETLINK_BUFFER_MAX_SIZE || length <= 0) {
        NETMGR_EXT_LOG_E("SendDataToKernel: length error");
        return ret;
    }
    uint32_t len = length + sizeof(nlmsghdr);
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(len);
    if (!buffer) {
        NETMGR_EXT_LOG_E("SendDataToKernel: malloc failed");
        return ret;
    }
    const auto &pnlmsg = reinterpret_cast<NetlinkInfo*>(buffer.get());

    if (memcpy_s(reinterpret_cast<char*>(pnlmsg->data), length, &msgData, length) != EOK) {
        NETMGR_EXT_LOG_E("SendDataToKernel: memcpy_s error");
        return ret;
    }
    ret = SendMsgToKernel(NETWORKSLICE_DATA_MSG, reinterpret_cast<nlmsghdr*>(pnlmsg), length + sizeof(nlmsghdr));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("SendDataToKernel :send msg failed");
    }
    NETMGR_EXT_LOG_I("SendDataToKernel:send msg success");
    return ret;
}

void NetworkSliceKernelProxy::StartRecvThread()
{
    taskQos_ = ffrt_this_task_get_qos();
    ffrt_epoll_ctl(taskQos_, EPOLL_CTL_ADD, netlinkSocket_, EPOLLIN, (void*)this,
        NetworkSliceKernelProxy::RecvKernelData);
    NETMGR_EXT_LOG_I("StartRecvThread success");
}

void NetworkSliceKernelProxy::StopRecvThread()
{
    ffrt_epoll_ctl(taskQos_, EPOLL_CTL_DEL, netlinkSocket_, 0, nullptr, nullptr);
    if (netlinkSocket_ >= 0) {
        close(netlinkSocket_);
        netlinkSocket_ = -1;
    }
    NETMGR_EXT_LOG_E("RecvThread: poll pthread_exit");
}

void NetworkSliceKernelProxy::RecvKernelData(void *data, uint32_t event)
{
    NETMGR_EXT_LOG_I("NetworkSliceKernelProxy RecvKernelData");
    if (data == nullptr) {
        return;
    }
    NetworkSliceKernelProxy *proxy = reinterpret_cast<NetworkSliceKernelProxy*>(data);

    if (proxy->netlinkSocket_ < 0) {
        NETMGR_EXT_LOG_E("Recvkerneldata: socket error!");
        return;
    }

    uint32_t len = NETLINK_BUFFER_MAX_SIZE + sizeof(nlmsghdr);
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(len);
    if (!buffer) {
        NETMGR_EXT_LOG_E("RecvKernelData: malloc failed");
        return;
    }
    const auto &pnlmsg = reinterpret_cast<NetlinkInfo*>(buffer.get());
    sockaddr_nl ntlAddr = {};
    ntlAddr.nl_family = AF_NETLINK;
    ntlAddr.nl_pid = 0;
    ntlAddr.nl_groups = 0;
    uint32_t ntlLen = sizeof(sockaddr_nl);
    
    int32_t totalLen = recvfrom(proxy->netlinkSocket_, pnlmsg, (NETLINK_BUFFER_MAX_SIZE + sizeof(nlmsghdr)), 0,
        (sockaddr *)&ntlAddr, (socklen_t *)&ntlLen);
    int32_t dataLen = totalLen - static_cast<int32_t>(sizeof(nlmsghdr));
    if (!proxy->IsValidDataLen(dataLen)) {
        NETMGR_EXT_LOG_E("RecvKernelData: dataLen is invalid, %{public}d", dataLen);
        return;
    }

    proxy->DispatchKernelMsg(&pnlmsg->data[0], dataLen);
}

void NetworkSliceKernelProxy::DispatchKernelMsg(void *msg, int32_t dataLen)
{
    NETMGR_EXT_LOG_I("NetworkSliceKernelProxy::DispatchKernelMsg datalen:%{public}d", dataLen);
    if (!msg) {
        NETMGR_EXT_LOG_E("NetworkSliceKernelProxy msg is null");
        return;
    }
    DelayedSingleton<NetworkSliceService>::GetInstance()->RecvKernelData(msg, dataLen);
}

bool NetworkSliceKernelProxy::IsValidDataLen(int32_t dataLen)
{
    return dataLen >= static_cast<int32_t>(sizeof(KernelMsg)) && dataLen <= NETLINK_BUFFER_MAX_SIZE;
}
} // namespace NetManagerStandard
} // namespace OHOS
