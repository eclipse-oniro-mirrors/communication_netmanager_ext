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

#include "net_manager_constants.h"
#include "router_advertisement_daemon.h"
#include <sys/time.h>

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MIN_RTR_ADV_INTERVAL_SEC = 300;
constexpr uint32_t MAX_RTR_ADV_INTERVAL_SEC = 600;
constexpr uint32_t TIME_MS_UNIT = 1000;
constexpr uint32_t SOCKET_SEND_TIMEOUT_MS = 300;
constexpr uint32_t DEFAULT_LIFETIME = 6 * MAX_RTR_ADV_INTERVAL_SEC;
constexpr uint32_t MIN_DELAY_BETWEEN_RAS_SEC = 3;
constexpr uint32_t DAY_IN_SECONDS = 24 * 60 * 60;
constexpr uint32_t MAX_URGENT_RTR_ADVERTISEMENTS = 10;
constexpr size_t RA_HEADER_SIZE = 16;
constexpr uint32_t HW_MAC_LENGTH = 6;
constexpr uint8_t DEFAULT_ROUTER_PRE = 0x08;
constexpr uint8_t HALF_IPV6_PREFIX = 64;
constexpr uint8_t PREFIX_LEN_TYPE_DEF = 1;
constexpr uint8_t PREFIX_LEN_TYPE_SMALL = 2;
constexpr uint8_t PREFIX_LEN_TYPE_BIG = 3;
constexpr uint8_t ROUTER_INFO_PRE = 0x18;
constexpr uint8_t PREFIX_OCTS_0 = 0;
constexpr uint8_t PREFIX_OCTS_1 = 8;
constexpr uint8_t PREFIX_OCTS_2 = 16;
constexpr uint8_t PREFIX_INFO_LEN = 4;
constexpr uint8_t PREFIX_INFO_FLAGS = 0xc0;
constexpr uint8_t BYTE_BIT = 8;
constexpr int32_t MAC_ADDRESS_STR_LEN = 18;
constexpr int32_t MAC_SSCANF_SPACE = 3;
constexpr uint32_t DEFAULT_HOP_LIMIT = 255;

/** ICMPv6 Ra package type, refer to
https://tools.ietf.org/html/rfc4861#section-13
*   Message name                            ICMPv6 Type
*   Router Solicitation                      133
*   Router Advertisement                     134

*  Option Name                               Type
*  Source Link-Layer Address                   1
*  Target Link-Layer Address                   2
*  Prefix Information                          3
*  MTU                                         5
*/
constexpr uint8_t ICMPV6_ND_ROUTER_SOLICIT_TYPE = 133;
constexpr uint8_t ICMPV6_ND_ROUTER_ADVERT_TYPE = 134;
constexpr uint8_t ND_OPTION_SLLA_TYPE = 1;
constexpr uint8_t ND_OPTION_PIO_TYPE = 3;
constexpr uint8_t ND_OPTION_MTU_TYPE = 5;

// https://datatracker.ietf.org/doc/html/rfc4191/#section-2.3
// 2.3.  Route Information Option
//        Type        24
constexpr uint8_t ND_OPTION_RIO_TYPE = 24;

// The length used to shift operation
constexpr int32_t SHIFT_8 = 8;
constexpr int32_t SHIFT_16 = 16;
constexpr int32_t SHIFT_24 = 24;
constexpr int32_t RA_REVERSE_LENGTH = 4;
} // namespace

RouterAdvertisementDaemon::RouterAdvertisementDaemon() : random_(std::random_device{}()), urgentAnnouncements_(0)
{
    raParams_ = std::make_shared<RaParams>();
}

bool RouterAdvertisementDaemon::IsSocketValid()
{
    return socket_ > 0;
}

void RouterAdvertisementDaemon::HupRaThread()
{
    stopRaThread_ = true;
}

int32_t RouterAdvertisementDaemon::Init(const std::string &ifaceName)
{
    // save iface param
    raParams_->name_ = ifaceName;
    raParams_->index_ = if_nametoindex(ifaceName.c_str());

    /** https://tools.ietf.org/html/rfc4861#section-4.2
     * Destination Address
     *              Typically the Source Address of an invoking Router
     *              Solicitation or the all-nodes multicast address.
     * https://tools.ietf.org/html/rfc4861#section-2.3
     * all-nodes multicast address
     *          - the link-local scope address to reach all nodes,
     *            FF02::1.
     */
    auto ret = memset_s(&dstIpv6Addr_, sizeof(dstIpv6Addr_), 0, sizeof(dstIpv6Addr_));
    if (ret != EOK) {
        return ret;
    }
    dstIpv6Addr_.sin6_port = htons(0);
    dstIpv6Addr_.sin6_family = AF_INET6;
    dstIpv6Addr_.sin6_scope_id = 0;
    inet_pton(AF_INET6, "ff02::1", &dstIpv6Addr_.sin6_addr);
    return NETMANAGER_EXT_SUCCESS;
}

bool RouterAdvertisementDaemon::StartRa()
{
    NETMGR_EXT_LOG_I("StartRa");
    if (!CreateRASocket()) {
        NETMGR_EXT_LOG_E("StartRa fail due to socket");
        return false;
    }
    sendRaThread_ = std::thread(&RouterAdvertisementDaemon::RunSendRaThread, this);
    pthread_setname_np(sendRaThread_.native_handle(), "OH_Net_SendRa");
    recvRaThread_ = std::thread(&RouterAdvertisementDaemon::RunRecvRsThread, this);
    pthread_setname_np(recvRaThread_.native_handle(), "OH_Net_RecvRs");
    sendRaThread_.detach();
    recvRaThread_.detach();
    return true;
}

void RouterAdvertisementDaemon::StopRa()
{
    NETMGR_EXT_LOG_I("StopRa");
    HupRaThread();
    CloseRaSocket();
    raParams_ = nullptr;
}

bool RouterAdvertisementDaemon::CreateRASocket()
{
    NETMGR_EXT_LOG_I("CreateRASocket Start");
    socket_ = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (socket_ < 0) {
        NETMGR_EXT_LOG_E("CreateRASocket fail, errno[%{public}d]", errno);
        return false;
    }

    timeval timeout = {};
    timeout.tv_sec = SOCKET_SEND_TIMEOUT_MS / TIME_MS_UNIT;
    timeout.tv_usec = (SOCKET_SEND_TIMEOUT_MS % TIME_MS_UNIT) * TIME_MS_UNIT;

    if (setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        NETMGR_EXT_LOG_E("CreateRASocket setsockopt SO_SNDTIMEO fail");
        close(socket_);
        return false;
    }

    ifreq ifr = {};
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ - 1, raParams_->name_.c_str(), raParams_->name_.size()) != EOK) {
        NETMGR_EXT_LOG_E("CreateRASocket strncopy fail");
        close(socket_);
        return false;
    }
    if (setsockopt(socket_, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        NETMGR_EXT_LOG_E("CreateRASocket setsockopt SO_BINDTODEVICE fail");
        close(socket_);
        return false;
    }

    uint32_t hoplimitNew = DEFAULT_HOP_LIMIT;
    if (setsockopt(socket_, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (void *)&hoplimitNew, sizeof(hoplimitNew)) == -1) {
        NETMGR_EXT_LOG_E(" setsockopt IPV6_UNICAST_HOPS fail");
    }

    return true;
}

void RouterAdvertisementDaemon::CloseRaSocket()
{
    NETMGR_EXT_LOG_I("CloseRaSocket Start");
    if (socket_ > 0) {
        close(socket_);
    }
    socket_ = -1;
}

bool RouterAdvertisementDaemon::MaybeSendRa(sockaddr_in6 &dest)
{
    NETMGR_EXT_LOG_I("Send Ra Enter, socket_[%{public}d], raPacketLength_[%{public}zu]", socket_, raPacketLength_);
    if (raPacketLength_ < RA_HEADER_SIZE) {
        NETMGR_EXT_LOG_E("Send Ra failed due to Ra packet length less than RA header size");
        return false;
    }
    if (!IsSocketValid()) {
        NETMGR_EXT_LOG_E("Send Ra failed due to socket invalid");
        return false;
    }

    int ret = sendto(socket_, raPacket_, raPacketLength_, 0, (sockaddr *)&dest, sizeof(dest));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("Send Ra error, ret[%{public}d], errno[%{public}d]", ret, errno);
        return false;
    }
    return true;
}

void RouterAdvertisementDaemon::RunRecvRsThread()
{
    NETMGR_EXT_LOG_I("Start to receive Rs thread, socket[%{public}d]", socket_);
    sockaddr_in6 solicitor = {};
    uint8_t solicitation[IPV6_MIN_MTU] = {};
    socklen_t sendLen = sizeof(solicitation);

    while (IsSocketValid()) {
        if (stopRaThread_) {
            break;
        }
        auto rval =
            recvfrom(socket_, solicitation, IPV6_MIN_MTU, 0, reinterpret_cast<sockaddr *>(&solicitor), &sendLen);
        if (rval < 1 || solicitation[0] != ICMPV6_ND_ROUTER_SOLICIT_TYPE) {
            if (rval <= 0 && errno != EAGAIN && errno != EINTR) {
                NETMGR_EXT_LOG_E("recvfrom failed, rval[%{public}d], errno[%{public}d]", rval, errno);
                break;
            } else {
                continue;
            }
        }
        AssembleRaLocked();
        MaybeSendRa(solicitor);
    }
}

void RouterAdvertisementDaemon::RunSendRaThread()
{
    NETMGR_EXT_LOG_I("Start send Ra thread, socket[%{publicd}d]", socket_);
    urgentAnnouncements_.store(MAX_URGENT_RTR_ADVERTISEMENTS - 1);
    while (IsSocketValid()) {
        if (stopRaThread_) {
            break;
        }
        if (AssembleRaLocked()) {
            MaybeSendRa(dstIpv6Addr_);
        }
        auto time = GetNextRaRetryIntervalMs();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
}

RaParams RouterAdvertisementDaemon::GetDeprecatedRaParams(RaParams &oldRa, RaParams &newRa)
{
    RaParams deprecateRa = {};
    for (auto ipp : newRa.prefixes_) {
        if (oldRa.ContainsPrefix(ipp)) {
            deprecateRa.prefixes_.emplace_back(ipp);
        }
    }
    for (auto dns : newRa.dnses_) {
        if (oldRa.ContainsDns(dns)) {
            deprecateRa.dnses_.emplace_back(dns);
        }
    }
    return deprecateRa;
}

void RouterAdvertisementDaemon::BuildNewRa(const RaParams &deprecatedRa, const RaParams &newRa)
{
    for (auto prefix : deprecatedRa.prefixes_) {
        deprecatedInfoTracker_.deprecatedPrefixes.emplace_back(prefix);
    }
    for (auto dns : deprecatedRa.dnses_) {
        deprecatedInfoTracker_.deprecatedDnses.emplace_back(dns);
    }
    raParams_->Set(newRa);
}

long RouterAdvertisementDaemon::GetNextRaRetryIntervalMs()
{
    int32_t nextRetansTimeSec = 0;
    int32_t urgentPending = urgentAnnouncements_.fetch_sub(1);
    if (raPacketLength_ < RA_HEADER_SIZE) {
        NETMGR_EXT_LOG_W("RaLength is smaller than min RA header size, wait one day");
        nextRetansTimeSec = DAY_IN_SECONDS;
    } else if (urgentPending > 0) {
        nextRetansTimeSec = MIN_DELAY_BETWEEN_RAS_SEC;
    } else {
        nextRetansTimeSec =
            MIN_RTR_ADV_INTERVAL_SEC + random_() % (MAX_RTR_ADV_INTERVAL_SEC - MIN_RTR_ADV_INTERVAL_SEC);
    }

    return TIME_MS_UNIT * static_cast<long>(nextRetansTimeSec);
}

bool RouterAdvertisementDaemon::AssembleRaLocked()
{
    NETMGR_EXT_LOG_D("Generate Ra package start");
    if (memset_s(&raPacket_, sizeof(raPacket_), 0, sizeof(raPacket_)) != EOK) {
        return false;
    }
    std::vector<uint8_t> raPackage;

    PutRaHeader(raPackage);
    if (!PutRaSlla(raPackage, raParams_->macAddr_)) {
        NETMGR_EXT_LOG_E("Add Ra Slla failed");
        return false;
    }

    PutRaMtu(raPackage, raParams_->mtu_);
    for (IpPrefix ipp : raParams_->prefixes_) {
        memset_s(ipp.address.s6_addr + BYTE_BIT, sizeof(ipp.address.s6_addr) - BYTE_BIT, 0,
                 sizeof(ipp.address.s6_addr) - BYTE_BIT);
        PutRaPio(raPackage, ipp, DEFAULT_LIFETIME, DEFAULT_LIFETIME);
    }

    raPacketLength_ = 0;
    if (raPackage.size() > IPV6_MIN_MTU) {
        NETMGR_EXT_LOG_E("Generate Ra package fail due to ra package too big: %{public}zu", raPackage.size());
        return false;
    }
    for (uint8_t byte : raPackage) {
        raPacket_[raPacketLength_++] = byte;
    }
    NETMGR_EXT_LOG_D("Generate Ra package end, raPacketLength_: %{public}zu", raPacketLength_);
    return true;
}

void RouterAdvertisementDaemon::PutRaHeader(std::vector<uint8_t> &buffer)
{
    NETMGR_EXT_LOG_D("Append Ra header");

    // https://datatracker.ietf.org/doc/html/rfc4861#section-4.2
    // 0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |     Code      |          Checksum             |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         Reachable Time                        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          Retrans Timer                        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |   Options ...
    // +-+-+-+-+-+-+-+-+-+-+-+-
    buffer.emplace_back(ICMPV6_ND_ROUTER_ADVERT_TYPE); // Type
    buffer.emplace_back(0);                            // Code
    buffer.emplace_back(0);                            // Checksum (placeholder)
    buffer.emplace_back(0);                            // Checksum (placeholder)
    buffer.emplace_back(DEFAULT_HOPLIMIT);             // Current hop limit

    // refer to https://tools.ietf.org/html/rfc4191#section-2.2
    buffer.emplace_back(DEFAULT_ROUTER_PRE); // Flags
    buffer.emplace_back(DEFAULT_LIFETIME >> SHIFT_8);
    buffer.emplace_back(DEFAULT_LIFETIME & 0xFF);
    buffer.insert(buffer.end(), RA_REVERSE_LENGTH, 0); // Reachable Time
    buffer.insert(buffer.end(), RA_REVERSE_LENGTH, 0); // Retrans Timer
}

bool RouterAdvertisementDaemon::PutRaSlla(std::vector<uint8_t> &buffer, const std::string &mac)
{
    NETMGR_EXT_LOG_D("Append Ra source link lay address");

    // https://datatracker.ietf.org/doc/html/rfc4861#section-4.6.1
    //  0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |    Length     |    Link-Layer Address ...
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    if (mac.size() < HW_MAC_LENGTH) {
        return false;
    }

    buffer.emplace_back(ND_OPTION_SLLA_TYPE); // Type
    buffer.emplace_back(1);                   // Length (in units of 8 octets)
    char strAddr[MAC_ADDRESS_STR_LEN] = {};
    if (memcpy_s(strAddr, MAC_ADDRESS_STR_LEN, mac.c_str(), mac.size()) != 0) {
        return false;
    }

    uint8_t byte = 0;
    for (uint32_t i = 0; i < HW_MAC_LENGTH; i++) {
        if (sscanf_s(strAddr + MAC_SSCANF_SPACE * i, "%2x", &byte) <= 0) {
            return false;
        }
        buffer.emplace_back(static_cast<uint8_t>(byte));
    }
    return true;
}

void RouterAdvertisementDaemon::PutRaMtu(std::vector<uint8_t> &buffer, int32_t mtu)
{
    NETMGR_EXT_LOG_D("Append Ra Mtu option");

    // https://datatracker.ietf.org/doc/html/rfc4861#section-4.6.4
    //    0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |    Length     |           Reserved            |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                              MTU                              |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    buffer.emplace_back(ND_OPTION_MTU_TYPE);
    buffer.emplace_back(1);
    buffer.emplace_back(0);
    buffer.emplace_back(0);
    buffer.emplace_back(static_cast<uint8_t>((mtu >> SHIFT_24) & 0xFF));
    buffer.emplace_back(static_cast<uint8_t>((mtu >> SHIFT_16) & 0xFF));
    buffer.emplace_back(static_cast<uint8_t>((mtu >> SHIFT_8) & 0xFF));
    buffer.emplace_back(static_cast<uint8_t>(mtu) & 0xFF);
}

void RouterAdvertisementDaemon::PutRaPio(std::vector<uint8_t> &buffer, const IpPrefix &ipp, int validTime,
                                         int preferredTime)
{
    NETMGR_EXT_LOG_D("Append Ra prefix information option");

    // refer to https://tools.ietf.org/html/rfc4861#section-4.6.2
    //    0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |    Length     | Prefix Length |L|A| Reserved1 |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         Valid Lifetime                        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                       Preferred Lifetime                      |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                           Reserved2                           |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                                                               |
    // +                                                               +
    // |                                                               |
    // +                            Prefix                             +
    // |                                                               |
    // +                                                               +
    // |                                                               |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    buffer.emplace_back(ND_OPTION_PIO_TYPE); // Type
    buffer.emplace_back(PREFIX_INFO_LEN);    // Length (4 = 32 octets)
    buffer.emplace_back(ipp.prefixesLength); // Prefix length
    buffer.emplace_back(PREFIX_INFO_FLAGS);  // L & A flags
    buffer.emplace_back((validTime >> SHIFT_24) & 0xFF);
    buffer.emplace_back((validTime >> SHIFT_16) & 0xFF);
    buffer.emplace_back((validTime >> SHIFT_8) & 0xFF);
    buffer.emplace_back(validTime & 0xFF);
    buffer.emplace_back((preferredTime >> SHIFT_24) & 0xFF);
    buffer.emplace_back((preferredTime >> SHIFT_16) & 0xFF);
    buffer.emplace_back((preferredTime >> SHIFT_8) & 0xFF);
    buffer.emplace_back(preferredTime & 0xFF);
    buffer.insert(buffer.end(), RA_REVERSE_LENGTH, 0); // Reserved
    for (uint8_t byte : ipp.address.s6_addr) {
        buffer.emplace_back(byte);
    }
}

void RouterAdvertisementDaemon::PutRaRio(std::vector<uint8_t> &buffer, const IpPrefix &ipp, int routeLifetime)
{
    NETMGR_EXT_LOG_D("Append Ra router information option");

    // refer to https://tools.ietf.org/html/rfc4191#section-2.3
    // 	0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |    Length     | Prefix Length |Resvd|Prf|Resvd|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                        Route Lifetime                         |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                   Prefix (Variable Length)                    |
    // .                                                               .
    // .                                                               .
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    int prefixLength = ipp.prefixesLength;
    int rioLength = PREFIX_LEN_TYPE_DEF;
    if (prefixLength == 0) {
        rioLength = PREFIX_LEN_TYPE_DEF;
    } else if (prefixLength > 0 && prefixLength <= HALF_IPV6_PREFIX) {
        rioLength = PREFIX_LEN_TYPE_SMALL;
    } else {
        rioLength = PREFIX_LEN_TYPE_BIG;
    }
    buffer.emplace_back(ND_OPTION_RIO_TYPE);
    buffer.emplace_back(rioLength);
    buffer.emplace_back(prefixLength);
    buffer.emplace_back(ROUTER_INFO_PRE);
    buffer.emplace_back((routeLifetime >> SHIFT_24) & 0xFF);
    buffer.emplace_back((routeLifetime >> SHIFT_16) & 0xFF);
    buffer.emplace_back((routeLifetime >> SHIFT_8) & 0xFF);
    buffer.emplace_back(routeLifetime & 0xFF);

    // refer to https://tools.ietf.org/html/rfc4191#section-2.3
    // The Prefix field is 0, 8, or 16 octets depending on Length.
    size_t bytesToWrite = PREFIX_OCTS_0;
    if (prefixLength == 0) {
        bytesToWrite = PREFIX_OCTS_0;
    } else if (prefixLength > 0 && prefixLength <= HALF_IPV6_PREFIX) {
        bytesToWrite = PREFIX_OCTS_1;
    } else {
        bytesToWrite = PREFIX_OCTS_2;
    }
    for (size_t i = 0; i < bytesToWrite; ++i) {
        buffer.emplace_back((ipp.address.s6_addr)[i]);
    }
}

} // namespace NetManagerStandard
} // namespace OHOS
