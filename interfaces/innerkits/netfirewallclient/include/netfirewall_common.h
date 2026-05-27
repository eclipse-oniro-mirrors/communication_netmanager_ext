/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NETFIREWALL_COMMON_H
#define NETFIREWALL_COMMON_H

#include <string>
#include <vector>
#include "parcel.h"
#include "netmgr_ext_log_wrapper.h"
#include "netfirewall_parcel.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
// Network firewall related specifications
// Maximum number of rules per user
constexpr int32_t FIREWALL_USER_MAX_RULE = 1000;
// Maximum number of pages per page during pagination queries
constexpr uint32_t MAX_PAGE_SIZE = 50;
// Maximum number of rules for all users
constexpr int32_t FIREWALL_ALL_USER_MAX_RULE = 2000;
// Maximum number of domain for per users
constexpr int32_t FIREWALL_SINGLE_USER_MAX_DOMAIN = 1000;
// Maximum number of domain for all users
constexpr int32_t FIREWALL_ALL_USER_MAX_DOMAIN = 2000;
// Maximum number of fuzzy domain for all users
constexpr int32_t FIREWALL_ALL_USER_MAX_FUZZY_DOMAIN = 100;
// Maximum length of rule name
constexpr int32_t MAX_RULE_NAME_LEN = 128;
// Maximum length of rule description
constexpr int32_t MAX_RULE_DESCRIPTION_LEN = 256;
// Maximum number of IPs per rule
constexpr int32_t MAX_RULE_IP_COUNT = 10;
// Maximum number of ports per rule
constexpr int32_t MAX_RULE_PORT_COUNT = 10;
// Maximum number of domain per rule
constexpr int32_t MAX_RULE_DOMAIN_COUNT = 100;
// Maximum exact domain name length
constexpr size_t MAX_EXACT_DOMAIN_NAME_LEN = 253;
// Maximum fuzzy domain name length
constexpr size_t MAX_FUZZY_DOMAIN_NAME_LEN = 63;
// Intercept log aging: maximum save time
constexpr int32_t RECORD_MAX_SAVE_TIME = 8 * 24 * 60 * 60;
// Intercept log aging: Save maximum number of entries
constexpr int32_t RECORD_MAX_DATA_NUM = 1000;

constexpr uint8_t IPV4_MASK_MAX = 32;
constexpr uint8_t IPV6_MASK_MAX = 128;

const std::string NET_FIREWALL_PAGE = "page";
const std::string NET_FIREWALL_PAGE_SIZE = "pageSize";
const std::string NET_FIREWALL_ORDER_FIELD = "orderField";
const std::string NET_FIREWALL_ORDER_TYPE = "orderType";
const std::string NET_FIREWALL_TOTAL_PAGE = "totalPage";
const std::string NET_FIREWALL_PAGE_DATA = "data";
}
// Traffic filter related constants
constexpr uint8_t NETTRAFFICFILTER_IP_ADDRLEN = 16;
constexpr uint8_t NETTRAFFICFILTER_MAX_MULTI_IP_COUNT = 16;
constexpr uint8_t NETTRAFFICFILTER_MAX_MULTI_PORT_COUNT = 64;
constexpr uint8_t NETTRAFFICFILTER_IFNAMSIZ = 32;
constexpr int32_t NETTRAFFICFILTER_MIN_GROUP_ID = 1;
constexpr int32_t NETTRAFFICFILTER_MAX_GROUP_ID = 65535;
constexpr int32_t NETTRAFFICFILTER_MIN_PRIORITY = 1;
constexpr int32_t NETTRAFFICFILTER_MAX_PRIORITY = 10000;
constexpr uint8_t NETTRAFFICFILTER_PROTO_ANY = 0;
constexpr uint8_t NETTRAFFICFILTER_PROTO_TCP = 6;
constexpr uint8_t NETTRAFFICFILTER_PROTO_UDP = 17;

enum class TrafficFilterIPFamily {
    IP_FAMILY_UNSPEC = 0,
    IP_FAMILY_V4 = 1,
    IP_FAMILY_V6 = 2,
    IP_FAMILY_V4V6
};

enum class TrafficFilterIPMatchType {
    IP_MATCH_ANY = 0,
    IP_MATCH_SINGLE,
    IP_MATCH_CIDR,
    IP_MATCH_RANGE,
    IP_MATCH_MULTI
};

enum class TrafficFilterPortMatchType {
    PORT_MATCH_ANY = 0,
    PORT_MATCH_SINGLE,
    PORT_MATCH_RANGE,
    PORT_MATCH_MULTI
};

enum class TrafficFilterHookPoint {
    HOOK_INPUT = 0,
    HOOK_OUTPUT = 1,
    HOOK_FORWARD = 2,
    HOOK_PREROUTING = 3,
    HOOK_POSTROUTING = 4
};

// Sort by rule name or interception time
enum class NetFirewallOrderField {
    ORDER_BY_RULE_NAME = 1,     // Sort by rule name
    ORDER_BY_RECORD_TIME = 100, // Sort by interception record time
};

// Paging query sorting enumeration
enum class NetFirewallOrderType {
    ORDER_ASC = 1,    // Ascending order
    ORDER_DESC = 100, // Descending order
};

// Firewall policy
struct NetFirewallPolicy : public Parcelable {
    bool isOpen;                  // Whether to open, required
    FirewallRuleAction inAction;  // Inbound default allowed or blocked, mandatory
    FirewallRuleAction outAction; // Outbound default allowed or blocked, mandatory

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetFirewallPolicy> Unmarshalling(Parcel &parcel);
};

// Pagination query input
struct RequestParam : public Parcelable {
    int32_t page;                     // Current page
    int32_t pageSize;                 // Page size
    NetFirewallOrderField orderField; // Sort Filed
    NetFirewallOrderType orderType;   // sort order
    std::string ToString() const;
    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<RequestParam> Unmarshalling(Parcel &parcel);
};

// Paging query results
struct FirewallRulePage : public Parcelable {
    int32_t page;                      // Current page
    int32_t pageSize;                  // Page size
    int32_t totalPage;                 // General page
    std::vector<NetFirewallRule> data; // Page data
    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<FirewallRulePage> Unmarshalling(Parcel &parcel);
};

// Intercept record pagination content
struct InterceptRecordPage : public Parcelable {
    int32_t page;                      // Current page
    int32_t pageSize;                  // Page size
    int32_t totalPage;                 // General page
    std::vector<InterceptRecord> data; // Page data
    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<InterceptRecordPage> Unmarshalling(Parcel &parcel);
};

// Traffic filter structures for binary IPC transmission
struct TrafficFilterIPAddress final : public Parcelable {
    int32_t family_;
    uint8_t addr_[16];

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterIPAddress> Unmarshalling(Parcel &parcel);
};

struct TrafficFilterIPCidr final : public Parcelable {
    TrafficFilterIPAddress base_;
    uint8_t prefixLen_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterIPCidr> Unmarshalling(Parcel &parcel);
};

struct TrafficFilterIPRange final : public Parcelable {
    TrafficFilterIPAddress start_;
    TrafficFilterIPAddress end_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterIPRange> Unmarshalling(Parcel &parcel);
};

struct TrafficFilterIPMulti final : public Parcelable {
    uint32_t ipCount_;
    TrafficFilterIPAddress ips_[16];

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterIPMulti> Unmarshalling(Parcel &parcel);
};

struct TrafficFilterIPMatch final : public Parcelable {
    int32_t type_;
    bool invert_;
    TrafficFilterIPAddress single_;
    TrafficFilterIPCidr cidr_;
    TrafficFilterIPRange range_;
    TrafficFilterIPMulti multi_;

    bool IsValidType() const
    {
        return type_ >= static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_ANY) &&
            type_ <= static_cast<int32_t>(TrafficFilterIPMatchType::IP_MATCH_MULTI);
    }

    std::string GetErrorInfo() const
    {
        return "Invalid IPMatch type: " + std::to_string(type_);
    }

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterIPMatch> Unmarshalling(Parcel &parcel);

private:
    bool UnmarshallingByType(Parcel &parcel);
};

struct TrafficFilterPortRange final : public Parcelable {
    uint16_t startPort_;
    uint16_t endPort_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterPortRange> Unmarshalling(Parcel &parcel);
};

struct TrafficFilterPortMulti final : public Parcelable {
    uint32_t portCount_;
    uint16_t ports_[64];

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterPortMulti> Unmarshalling(Parcel &parcel);
};

struct TrafficFilterPortMatch final : public Parcelable {
    int32_t type_;
    bool invert_;
    uint16_t single_;
    TrafficFilterPortRange range_;
    TrafficFilterPortMulti multi_;

    bool IsValidType() const
    {
        return type_ >= static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_ANY) &&
            type_ <= static_cast<int32_t>(TrafficFilterPortMatchType::PORT_MATCH_MULTI);
    }

    std::string GetErrorInfo() const
    {
        return "Invalid PortMatch type: " + std::to_string(type_);
    }

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterPortMatch> Unmarshalling(Parcel &parcel);

private:
    bool UnmarshallingByType(Parcel &parcel);
};

struct TrafficFilterInterfaceMatch final : public Parcelable {
    bool enabled_;
    bool invert_;
    bool isPrefix_;
    std::string ifName_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterInterfaceMatch> Unmarshalling(Parcel &parcel);
};

struct TrafficFilterRedirectRule final : public Parcelable {
    uint32_t priority_;
    int32_t hookPoint_;
    uint8_t protocol_;
    TrafficFilterIPMatch srcIp_;
    TrafficFilterPortMatch srcPort_;
    TrafficFilterIPMatch dstIp_;
    TrafficFilterPortMatch dstPort_;
    TrafficFilterInterfaceMatch inInterface_;
    TrafficFilterInterfaceMatch outInterface_;
    uint32_t uidStart_;
    uint32_t uidEnd_;
    TrafficFilterIPAddress proxyIp_;
    uint16_t proxyPort_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<TrafficFilterRedirectRule> Unmarshalling(Parcel &parcel);

private:
    bool MarshallingUidAndProxyFields(Parcel &parcel) const;
    bool UnmarshalMatchAndInterfaceFields(Parcel &parcel);
};

inline uint64_t GetCurrentMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETFIREWALL_COMMON_H