/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ROUTER_ADVERTISEMENT_DAEMON_H
#define ROUTER_ADVERTISEMENT_DAEMON_H

#include "netmgr_ext_log_wrapper.h"
#include "router_advertisement_params.h"
#include <any>
#include <arpa/inet.h>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <net/if.h>
#include <netinet/in.h>
#include <random>
#include <securec.h>
#include <set>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace OHOS {
namespace NetManagerStandard {
struct DeprecatedInfoTracker {
    std::vector<IpPrefix> deprecatedPrefixes;
    std::vector<in6_addr> deprecatedDnses;
};

class RouterAdvertisementDaemon {
public:
    RouterAdvertisementDaemon();
    ~RouterAdvertisementDaemon() = default;
    int32_t Init(const std::string &ifaceName);
    int32_t InitDstIpv6Address();
    bool GetInterfaceParamInfo(const std::string &ifaceName);
    int32_t ConfigureAllNodes();
    bool StartRa();
    void StopRa();
    bool IsSocketValid();
    RaParams GetDeprecatedRaParams(RaParams &oldRa, RaParams &newRa);
    void BuildNewRa(const RaParams &deprecatedRa, const RaParams &newRa);

private:
    bool CreateRASocket();
    void CloseRaSocket();
    void RunSendRaThread();
    void RunRecvRsThread();
    void HupRaThread();
    bool MaybeSendRa(sockaddr_in6 &ra);
    long GetNextRaRetryIntervalMs();
    bool AssembleRaLocked();
    void PutRaHeader(std::vector<uint8_t> &buffer);
    void PutNaHeader(std::vector<uint8_t> &buffer);
    bool PutRaSlla(std::vector<uint8_t> &buffer, const std::string &mac);
    void PutRaExpandedFlagsOption(std::vector<uint8_t> &buffer);
    void PutRaMtu(std::vector<uint8_t> &buffer, int32_t mtu);
    void PutRaPio(std::vector<uint8_t> &buffer, const IpPrefix &ipp, int validTime, int preferredTime);
    void PutRaRio(std::vector<uint8_t> &buffer, const IpPrefix &ipp, int routeLifetime);
    bool PutRaRdnss(std::vector<uint8_t> &buffer, const std::vector<in6_addr> &dnses, int lifetime);

private:
    sockaddr_in6 dstIpv6Addr_ = {};
    int socket_ = -1;
    std::mt19937 random_;
    std::atomic<int> urgentAnnouncements_;
    std::mutex mutex_;
    std::thread sendRaThread_;
    std::thread recvRaThread_;
    volatile bool stopRaThread_ = false;
    uint8_t raPacket_[IPV6_MIN_MTU] = {};
    size_t raPacketLength_ = 0;
    std::shared_ptr<RaParams> raParams_;
    DeprecatedInfoTracker deprecatedInfoTracker_;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ROUTER_ADVERTISEMENT_DAEMON_H
