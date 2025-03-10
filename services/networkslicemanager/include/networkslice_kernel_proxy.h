/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_NETWORKSLICE_KERNEL_PROXY_H
#define OHOS_NETWORKSLICE_KERNEL_PROXY_H

#include <cstdint>
#include <shared_mutex>
#include <unordered_map>
#include <set>
#include <vector>
#include <linux/netlink.h>
#include "networkslice_service_base.h"
#include "singleton.h"
#include "ffrt_inner.h"
namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t NETLINK_SOCKET_DEFAULT = -1;
}

enum KernelReqMsgType {
    KERNEL_MSG_APP_QOE_SYNC_CMD = 0,
    KERNEL_MSG_UPDATE_APP_INFO_CMD = 1,
    KERNEL_MSG_TCP_PKT_COLLEC_CMD = 6,
    KERNEL_MSG_ICMP_PING_DETECT_CMD = 11,
    KERNEL_MSG_WIFI_PARA_COLLECT_START = 20,
    KERNEL_MSG_WIFI_PARA_COLLECT_STOP = 21,
    KERNEL_MSG_WIFI_PARA_COLLECT_UPDATE = 27,
    KERNEL_MSG_TRAFFIC_DATA_NETWORK_CHANGE = 34,
    KERNEL_MSG_TRAFFIC_UIDS = 35,
    KERNEL_MSG_FG_CHANGE_UID = 44,
    KERNEL_MSG_SOCKET_CLOSE_DETECT_UID_ADD = 46,
    KERNEL_MSG_SOCKET_CLOSE_DETECT_UID_DEL = 47,
    KERNEL_MSG_APP_ACCELERATOR_CMD = 51,

    // QOE 模块使用 100 ~200
    K_MSG_QOE_STREAM_CFG = 100,
    K_MSG_QOE_STREAM_HOOK_ADD = 101,
    K_MSG_QOE_STREAM_HOOK_DEL = 102
};

struct KernelIpRptEnableMsg {
    int16_t type; // Event enumeration values
    int16_t len; // The length behind this field, the limit lower 2048
    int16_t isEnable;
};

struct KernelBindMsg {
    int16_t type; // Event enumeration values
    int16_t len; // The length behind this field, the limit lower 2048
    unsigned char buf[0];
};

enum KernelRsqMsgType {
    KERNEL_RSP_APP_QOE = 0,
    KERNEL_RSP_FG_UID_DROP = 1,
    KERNEL_RSP_FG_UID_RECOVERY = 2,
    KERNEL_RSP_TCP_PKT_CONUT = 3,
    KERNEL_RSP_SLICE_IP_PARA = 4,
    KERNEL_RSP_ICMP_PING_REPORT = 5,
    KERNEL_RSP_PACKET_DELAY = 6,
    KERNEL_RSP_FASTGRAB_CHR = 7,
    KERNEL_RSP_STEADY_SPEED_STATS = 8,
    KERNEL_RSP_WIFI_PARA = 9,
    KERNEL_RSP_SPEED_TEST_CHR = 10,
    KERNEL_RSP_STREAM_DETECTION,
    KERNEL_RSP_TRAFFIC_STATS_INFO = 14,
    KERNEL_RSP_IPV6_SYNC_ABNORMAL_REPORT_UID = 16,
    KERNEL_RSP_SOCKET_CLOSE_CHR_MSG_ID = 17,
    KERNEL_RSP_TCP_RESET_CHR_MSG_ID = 18,
    KERNEL_RSP_APP_PROXY_RESULT = 19,
    KERNEL_RSP_TOP_APP_ABN_STAT_REPORT = 20,
    KERNEL_RSP_NBMSG_RPT_BUTT
};

struct NetlinkInfo {
    struct nlmsghdr hdr;
    unsigned char data[0];
};

/* Each module sends the message request is defined as: */
struct KernelMsg {
    int16_t type; // Event enumeration values
    int16_t len; // The length behind this field, the limit lower 2048
    char buf[0];
};

struct KernelMsgNS {
    char buf[0];
};

struct KernelCmdMsg {
    int16_t cmd;
    int16_t cnt;
    int32_t interval;
};

class NetworkSliceKernelProxy {
    DECLARE_DELAYED_SINGLETON(NetworkSliceKernelProxy);
public:
    void RegistHandler(NetworkSliceSubModule moduleId, NetworkSliceServiceBase* handler,
        const std::vector<int16_t>& msgTypeVec);
    void UnRegistHandler(NetworkSliceSubModule moduleId);
    int32_t StartNetlink();
    void StartRecvThread();
    void StopRecvThread();
    int32_t SendDataToKernel(KernelMsg &msgData);
private:
    int32_t NetlinkInit();
    int32_t SendMsgToKernel(int32_t type, nlmsghdr *nlmsg, size_t nlmsgLen);
    void RecvThread();
    static void RecvKernelData(void *data, uint32_t event);
    bool IsValidDataLen(int32_t dataLen);
    void DispatchKernelMsg(void *msg, int32_t dataLen);

    int32_t netlinkSocket_ { NETLINK_SOCKET_DEFAULT };
    std::unordered_map<NetworkSliceSubModule, NetworkSliceServiceBase*> moduleIdHandlerMap_ {};
    std::unordered_map<int16_t, std::set<NetworkSliceSubModule>> msgTypeModuleIdsMap_ {};
    std::unordered_map<NetworkSliceSubModule, std::vector<int16_t>> moduleIdMsgTypesMap_ {};
    std::shared_timed_mutex mutex_ {};
    ffrt_qos_t taskQos_ = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif
