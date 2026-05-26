/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/**
 * @addtogroup TrafficFilter
 * @{
 *
 * @brief Traffic filter and redirection subsystem - common types and error codes definition.
 *
 * @since 26.0.0
 */

/**
 * @file net_trafficfilter_type.h
 * @brief Traffic filter and redirection subsystem - common types and error codes definition.
 *
 * @library libnet_trafficfilter.so
 * @kit NetworkKit
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @syscap SystemCapability.Communication.NetManager.NetFirewall
 * @since 26.0.0
 */

#ifndef NET_TRAFFICFILTER_TYPE_H
#define NET_TRAFFICFILTER_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum length of IP address (compatible with IPv4 and IPv6)
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_IP_ADDRLEN       16

/**
 * @brief Maximum number of IPs supported in multi-IP matching
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MAX_MULTI_IP_COUNT  16

/**
 * @brief Maximum number of ports supported in multi-port matching
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MAX_MULTI_PORT_COUNT 64

/**
 * @brief NFQueue packet copy mode: copy metadata only
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_NFQUEUE_COPY_META   0

/**
 * @brief NFQueue packet copy mode: copy entire packet
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_NFQUEUE_COPY_PACKET 0xFFFF

/**
 * @brief Default NFQueue packet copy length in bytes
 * Set to 0xFFFF to copy the entire packet, smaller values (e.g., 128) copy only the packet header
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_DEFAULT_COPY_LEN    0xFFFF

/**
 * @brief Default NFQueue maximum queue length (number of packets)
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_DEFAULT_QUEUE_MAXLEN  1024

/**
 * @brief NFQueue queue flag: FAIL-OPEN mode
 * When userspace process crashes, kernel automatically accepts packets to avoid network interruption
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_NFQUEUE_FLAG_FAIL_OPEN  0x1

/**
 * @brief Maximum length of process name
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MAX_PROCESS_NAME_LEN 64

/**
 * @brief Maximum length of MAC address string (XX:XX:XX:XX:XX:XX)
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MAC_ADDRSTRLEN       18

/**
 * @brief Minimum priority value
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MIN_PRIORITY        1

/**
 * @brief Maximum priority value
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MAX_PRIORITY        10000

/**
 * @brief Minimum Group ID value
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MIN_GROUP_ID        1

/**
 * @brief Maximum Group ID value
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_MAX_GROUP_ID        65535

/**
 * @brief Maximum length of interface name
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_IFNAMSIZ            32

/**
 * @brief Protocol type constants
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_PROTO_ANY           0
#define OH_TRAFFICFILTER_PROTO_TCP           6
#define OH_TRAFFICFILTER_PROTO_UDP           17
#define OH_TRAFFICFILTER_PROTO_ICMP          1
#define OH_TRAFFICFILTER_PROTO_ICMPV6        58

/**
 * @brief TCP flag
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_TCP_FLAG_SYN        0x01
#define OH_TRAFFICFILTER_TCP_FLAG_ACK        0x02
#define OH_TRAFFICFILTER_TCP_FLAG_FIN        0x04
#define OH_TRAFFICFILTER_TCP_FLAG_RST        0x08
#define OH_TRAFFICFILTER_TCP_FLAG_PSH        0x10
#define OH_TRAFFICFILTER_TCP_FLAG_URG        0x20
#define OH_TRAFFICFILTER_TCP_FLAG_ALL        0x3F
#define OH_TRAFFICFILTER_TCP_FLAG_NONE       0x00

/**
 * @brief Connection state bitmap
 * @since 26.0.0
 */
#define OH_TRAFFICFILTER_CT_STATE_ANY         0x00
#define OH_TRAFFICFILTER_CT_STATE_NEW         0x01
#define OH_TRAFFICFILTER_CT_STATE_ESTABLISHED 0x02
#define OH_TRAFFICFILTER_CT_STATE_RELATED     0x04
#define OH_TRAFFICFILTER_CT_STATE_INVALID     0x08
#define OH_TRAFFICFILTER_CT_STATE_UNTRACKED   0x10

/**
 * @brief Traffic filter and redirection error codes
 * @since 26.0.0
 */
typedef enum OH_TrafficFilter_ErrCode {
    /**
     * @brief Operation successful
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_OK = 0,

    /**
     * @brief Error code base value
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_E_BASE = 29410000,

    /**
     * @brief Permission denied
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED = 201,

    /**
     * @brief Invalid parameter (invalid priority, IP address, port, or group ID)
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_ERROR_INVALID_PARAM = (OH_TRAFFICFILTER_E_BASE + 101),

    /**
     * @brief Resource not found (rule, target, process, or group ID not found)
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_ERROR_NOT_FOUND = (OH_TRAFFICFILTER_E_BASE + 102),

    /**
     * @brief Too many rules
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_ERROR_TOO_MANY_RULES = (OH_TRAFFICFILTER_E_BASE + 103),

    /**
     * @brief Group ID already in use
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_ERROR_GROUP_ID_IN_USE = (OH_TRAFFICFILTER_E_BASE + 104),

    /**
     * @brief NFQueue error (initialization failed or no available queue)
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR = (OH_TRAFFICFILTER_E_BASE + 105),
} OH_TrafficFilter_ErrCode;

/**
 * @brief IP match type
 * @since 26.0.0
 */
typedef enum OH_TrafficFilter_IPMatchType {
    /**
     * @brief Any IP
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_MATCH_ANY = 0,
    /**
     * @brief Single IP
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_MATCH_SINGLE,
    /**
     * @brief CIDR format
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_MATCH_CIDR,
    /**
     * @brief IP range
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_MATCH_RANGE,
    /**
     * @brief Multiple IPs
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_MATCH_MULTI
} OH_TrafficFilter_IPMatchType;

/**
 * @brief IP address family
 * @since 26.0.0
 */
typedef enum OH_TrafficFilter_IPFamily {
    /**
     * @brief Unspecified IP family
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_FAMILY_UNSPEC = 0,
    /**
     * @brief IPv4 address family
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_FAMILY_V4 = 1,
    /**
     * @brief IPv6 address family
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_IP_FAMILY_V6 = 2
} OH_TrafficFilter_IPFamily;

/**
 * @brief Port match type
 * @since 26.0.0
 */
typedef enum OH_TrafficFilter_PortMatchType {
    /**
     * @brief Any port
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_PORT_MATCH_ANY = 0,
    /**
     * @brief Single port
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_PORT_MATCH_SINGLE,
    /**
     * @brief Port range
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_PORT_MATCH_RANGE,
    /**
     * @brief Multiple port
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_PORT_MATCH_MULTI
} OH_TrafficFilter_PortMatchType;

/**
 * @brief Hook point type
 * @since 26.0.0
 */
typedef enum OH_TrafficFilter_HookPoint {
    /**
     * @brief INPUT chain
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_HOOK_INPUT = 0,
    /**
     * @brief OUTPUT chain
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_HOOK_OUTPUT,
    /**
     * @brief FORWARD chain
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_HOOK_FORWARD,
    /**
     * @brief PREROUTING chain
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_HOOK_PREROUTING,
    /**
     * @brief POSTROUTING chain
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_HOOK_POSTROUTING
} OH_TrafficFilter_HookPoint;

/**
 * @brief IP address in binary form, supports both IPv4 and IPv6
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_IPAddress {
    /**
     * @brief Address family, If not explicitly set, IPv4 is used by default.
     * @since 26.0.0
     */
    OH_TrafficFilter_IPFamily family;
    /**
     * @brief IP address bytes, uses first 4 bytes for IPv4 and all 16 bytes for IPv6
     * @since 26.0.0
     */
    uint8_t addr[OH_TRAFFICFILTER_IP_ADDRLEN];
} OH_TrafficFilter_IPAddress;

/**
 * @brief IP match value for CIDR match
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_IPCidr {
    /**
     * @brief Base IP address of the CIDR block
     * @since 26.0.0
     */
    OH_TrafficFilter_IPAddress base;
    /**
     * @brief CIDR prefix length
     * @since 26.0.0
     */
    uint8_t prefixLen;
} OH_TrafficFilter_IPCidr;

/**
 * @brief IP match value for range match
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_IPRange {
    /**
     * @brief Start IP address of the range
     * @since 26.0.0
     */
    OH_TrafficFilter_IPAddress start;
    /**
     * @brief End IP address of the range
     * @since 26.0.0
     */
    OH_TrafficFilter_IPAddress end;
} OH_TrafficFilter_IPRange;

/**
 * @brief IP match value for multi-IP match
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_IPMulti {
    /**
     * @brief Number of IP addresses in the array
     * @since 26.0.0
     */
    uint32_t ipCount;
    /**
     * @brief IP address array
     * @since 26.0.0
     */
    OH_TrafficFilter_IPAddress ips[OH_TRAFFICFILTER_MAX_MULTI_IP_COUNT];
} OH_TrafficFilter_IPMulti;

/**
 * @brief IP match condition
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_IPMatch {
    /**
     * @brief Match type
     * @since 26.0.0
     */
    OH_TrafficFilter_IPMatchType type;
    /**
     * @brief Whether to invert the match result
     * @since 26.0.0
     */
    bool invert;
    /**
     * @brief Match rule
     * @since 26.0.0
     */
    union {
        /**
         * @brief Single IP address, used when type is OH_TRAFFICFILTER_IP_MATCH_SINGLE
         * @since 26.0.0
         */
        OH_TrafficFilter_IPAddress single;
        /**
         * @brief CIDR match value, used when type is OH_TRAFFICFILTER_IP_MATCH_CIDR
         * @since 26.0.0
         */
        OH_TrafficFilter_IPCidr cidr;
        /**
         * @brief IP range match value, used when type is OH_TRAFFICFILTER_IP_MATCH_RANGE
         * @since 26.0.0
         */
        OH_TrafficFilter_IPRange range;
        /**
         * @brief Multi-IP match value, used when type is OH_TRAFFICFILTER_IP_MATCH_MULTI
         * @since 26.0.0
         */
        OH_TrafficFilter_IPMulti multi;
    } value;
} OH_TrafficFilter_IPMatch;

/**
 * @brief interface match condition
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_InterfaceMatch {
    /**
     * @brief Whether interface matching is enabled
     * @since 26.0.0
     */
    bool enabled;
    /**
     * @brief Whether to invert the match result
     * @since 26.0.0
     */
    bool invert;
    /**
     * @brief Whether the interface name is matched by prefix
     * @since 26.0.0
     */
    bool isPrefix;
    /**
     * @brief Interface name
     * @since 26.0.0
     */
    char ifName[OH_TRAFFICFILTER_IFNAMSIZ];
} OH_TrafficFilter_InterfaceMatch;

/**
 * @brief Port match value for range match
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_PortRange {
    /**
     * @brief Start port of the range
     * @since 26.0.0
     */
    uint16_t startPort;
    /**
     * @brief End port of the range
     * @since 26.0.0
     */
    uint16_t endPort;
} OH_TrafficFilter_PortRange;

/**
 * @brief Port match value for multi-port match
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_PortMulti {
    /**
     * @brief Number of ports in the array
     * @since 26.0.0
     */
    uint32_t portCount;
    /**
     * @brief Port array
     * @since 26.0.0
     */
    uint16_t ports[OH_TRAFFICFILTER_MAX_MULTI_PORT_COUNT];
} OH_TrafficFilter_PortMulti;

/**
 * @brief Port match condition
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_PortMatch {
    /**
     * @brief Match type
     * @since 26.0.0
     */
    OH_TrafficFilter_PortMatchType type;
    /**
     * @brief Whether to invert the match result
     * @since 26.0.0
     */
    bool invert;
    /**
     * @brief Match rule
     * @since 26.0.0
     */
    union {
        /**
         * @brief Single port, used when type is OH_TRAFFICFILTER_PORT_MATCH_SINGLE
         * @since 26.0.0
         */
        uint16_t single;
        /**
         * @brief Port range match value, used when type is OH_TRAFFICFILTER_PORT_MATCH_RANGE
         * @since 26.0.0
         */
        OH_TrafficFilter_PortRange range;
        /**
         * @brief Multi-port match value, used when type is OH_TRAFFICFILTER_PORT_MATCH_MULTI
         * @since 26.0.0
         */
        OH_TrafficFilter_PortMulti multi;
    } value;
} OH_TrafficFilter_PortMatch;

/**
 * @brief MAC address match condition
 *
 * Matches packets based on MAC address
 * Only source MAC is supported
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_MACMatch {
    /**
     * @brief Enable MAC address matching
     * @since 26.0.0
     */
    bool enable;
    /**
     * @brief Whether to invert the match result
     * @since 26.0.0
     */
    bool invert;
    /**
     * @brief Source MAC address (XX:XX:XX:XX:XX:XX format)
     * @since 26.0.0
     */
    char src_mac[OH_TRAFFICFILTER_MAC_ADDRSTRLEN];
} OH_TrafficFilter_MACMatch;

/**
 * @brief TCP flags match condition
 *
 * Matches TCP packets based on TCP flag settings
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_TCPFlagsMatch {
    /**
     * @brief Enable TCP flags matching
     * @since 26.0.0
     */
    bool enable;
    /**
     * @brief Flag mask (which flags to check, use OH_TRAFFICFILTER_TCP_FLAG_* constants)
     * @since 26.0.0
     */
    uint8_t flag_mask;
    /**
     * @brief Flag to compare (which flags must be set)
     * @since 26.0.0
     */
    uint8_t flag_comp;
} OH_TrafficFilter_TCPFlagsMatch;

/**
 * @brief Connection tracking match condition
 *
 * Matches packets based on connection tracking states
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_ConntrackMatch {
    /**
     * @brief Enable conntrack matching
     * @since 26.0.0
     */
    bool enable;
    /**
     * @brief Connection states (use OH_TRAFFICFILTER_CT_STATE_* bitmap)
     * @since 26.0.0
     */
    uint8_t state_mask;
} OH_TrafficFilter_ConntrackMatch;

/**
 * @brief NFQueue configuration structure
 *
 * This structure only contains NFQueue queue configuration, excluding application layer configuration
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_Config {
    /**
     * @brief NFQueue packet copy length in bytes, 0xFFFF means entire packet, smaller values copy only header
     * @since 26.0.0
     */
    uint32_t packet_copy_len;
    /**
     * @brief NFQueue maximum queue length (number of packets), 0 means system default (1024)
     * @since 26.0.0
     */
    uint32_t nfqueue_maxlen;
    /**
     * @brief NFQueue queue flags, see OH_TRAFFICFILTER_NFQUEUE_FLAG_* definitions
     * @since 26.0.0
     */
    uint32_t nfqueue_flags;
} OH_TrafficFilter_Config;

/**
 * @brief Connection information structure
 *
 * Describes five-tuple connection information used to query process information.
 *
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_ConnectionInfo {
    /**
     * @brief Source IP address, supports IPv4 and IPv6.
     * @since 26.0.0
     */
    const char* src_ip;
    /**
     * @brief Source port. 0 means any source port.
     * @since 26.0.0
     */
    uint16_t src_port;
    /**
     * @brief Destination IP address, supports IPv4 and IPv6.
     * @since 26.0.0
     */
    const char* dst_ip;
    /**
     * @brief Destination port. 0 means any destination port.
     * @since 26.0.0
     */
    uint16_t dst_port;
    /**
     * @brief Protocol type.
     * Supported values:
     * - OH_TRAFFICFILTER_PROTO_TCP (6)
     * - OH_TRAFFICFILTER_PROTO_UDP (17)
     * @since 26.0.0
     */
    uint8_t protocol;
} OH_TrafficFilter_ConnectionInfo;

/**
 * @brief Process information structure
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_ProcessInfo {
    /**
     * @brief Process ID
     * @since 26.0.0
     */
    uint32_t pid;
    /**
     * @brief User ID
     * @since 26.0.0
     */
    uint32_t uid;
} OH_TrafficFilter_ProcessInfo;

/**
 * @brief Packet controller
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_PacketController OH_TrafficFilter_PacketController;

/**
 * @brief Packet decision type
 * @since 26.0.0
 */
typedef enum OH_TrafficFilter_PacketDecision {
    /**
     * @brief Accept packet
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_DECISION_ACCEPT = 0,
    /**
     * @brief Drop packet
     * @since 26.0.0
     */
    OH_TRAFFICFILTER_DECISION_DROP
} OH_TrafficFilter_PacketDecision;

/**
 * @brief Packet descriptor
 *
 * Contains five-tuple information and packet data
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_PacketDesc {
    /**
     * @brief Packet ID (assigned by kernel when packet arrives at netfilter)
     * @since 26.0.0
     */
    uint32_t packet_id;
    /**
     * @brief Protocol type
     * @since 26.0.0
     */
    uint8_t protocol;
    /**
     * @brief Source IP address (supports IPv4 and IPv6)
     * @since 26.0.0
     */
    OH_TrafficFilter_IPAddress src_ip;
    /**
     * @brief Source port
     * @since 26.0.0
     */
    uint16_t src_port;
    /**
     * @brief Destination IP address (supports IPv4 and IPv6)
     * @since 26.0.0
     */
    OH_TrafficFilter_IPAddress dst_ip;
    /**
     * @brief Destination port
     * @since 26.0.0
     */
    uint16_t dst_port;
    /**
     * @brief Packet length
     * @since 26.0.0
     */
    uint32_t packet_len;
    /**
     * @brief Packet data pointer (user can modify, memory managed by system, valid only during callback)
     * @since 26.0.0
     */
    uint8_t* data;
    /**
     * @brief User data (used in callback)
     * @since 26.0.0
     */
    void* user_data;
} OH_TrafficFilter_PacketDesc;

/**
 * @brief Packet callback function type
 *
 * @param packet Packet descriptor
 * @param user_data User data
 * @return Packet decision (ACCEPT or DROP)
 * @since 26.0.0
 */
typedef OH_TrafficFilter_PacketDecision (*OH_TrafficFilter_PacketCallback)(
    const OH_TrafficFilter_PacketDesc* packet,
    void* user_data
);

/**
 * @brief Packet filter rule
 *
 * Defines conditions for matching packets
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_FilterRule {
    /**
     * @brief Priority (smaller number means higher priority)
     * @since 26.0.0
     */
    uint32_t priority;
    /**
     * @brief Hook point
     * @since 26.0.0
     */
    OH_TrafficFilter_HookPoint hook_point;
    /**
     * @brief Protocol (0=any, 6=TCP, 17=UDP)
     * @since 26.0.0
     */
    uint8_t protocol;
    /**
     * @brief Source IP match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_IPMatch src_ip;
    /**
     * @brief Source port match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_PortMatch src_port;
    /**
     * @brief Destination IP match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_IPMatch dst_ip;
    /**
     * @brief Destination port match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_PortMatch dst_port;
    /**
     * @brief Incoming interface match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_InterfaceMatch in_interface;
    /**
     * @brief Outgoing interface match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_InterfaceMatch out_interface;
    /**
     * @brief Application UID range start (UINT32_MAX means any)
     * @since 26.0.0
     */
    uint32_t uid_start;
    /**
     * @brief Application UID range end (UINT32_MAX means any)
     * @since 26.0.0
     */
    uint32_t uid_end;
    /**
     * @brief MAC address match condition (only source MAC)
     * @since 26.0.0
     */
    OH_TrafficFilter_MACMatch mac_match;
    /**
     * @brief TCP flags match condition (valid only for TCP protocol)
     * @since 26.0.0
     */
    OH_TrafficFilter_TCPFlagsMatch tcp_flags_match;
    /**
     * @brief Connection tracking match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_ConntrackMatch conntrack_match;
} OH_TrafficFilter_FilterRule;

/**
 * @brief Traffic redirector
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_Redirector OH_TrafficFilter_Redirector;

/**
 * @brief Traffic redirection rule
 *
 * Defines TCP traffic redirection rule to redirect matched traffic to specified proxy server
 * @since 26.0.0
 */
typedef struct OH_TrafficFilter_RedirectRule {
    /**
     * @brief Priority (smaller number means higher priority, same rule as packet filter)
     * @since 26.0.0
     */
    uint32_t priority;
    /**
     * @brief Hook point (only PREROUTING and OUTPUT are supported)
     * @since 26.0.0
     */
    OH_TrafficFilter_HookPoint hook_point;
    /**
     * @brief Protocol (fixed to TCP=6)
     * @since 26.0.0
     */
    uint8_t protocol;
    /**
     * @brief Source IP match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_IPMatch src_ip;
    /**
     * @brief Source port match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_PortMatch src_port;
    /**
     * @brief Destination IP match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_IPMatch dst_ip;
    /**
     * @brief Destination port match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_PortMatch dst_port;
    /**
     * @brief Incoming interface match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_InterfaceMatch in_interface;
    /**
     * @brief Outgoing interface match condition
     * @since 26.0.0
     */
    OH_TrafficFilter_InterfaceMatch out_interface;
    /**
     * @brief Application UID range start (UINT32_MAX means any)
     * @since 26.0.0
     */
    uint32_t uid_start;
    /**
     * @brief Application UID range end (UINT32_MAX means any)
     * @since 26.0.0
     */
    uint32_t uid_end;
    /**
     * @brief Proxy server IP address (supports IPv4 and IPv6)
     * @since 26.0.0
     */
    OH_TrafficFilter_IPAddress proxy_ip;
    /**
     * @brief Proxy server port
     * @since 26.0.0
     */
    uint16_t proxy_port;
} OH_TrafficFilter_RedirectRule;

#ifdef __cplusplus
}
#endif

#endif /* NET_TRAFFICFILTER_TYPE_H */
/** @} */
