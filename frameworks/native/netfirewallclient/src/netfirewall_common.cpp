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

#include <sstream>

#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "netfirewall_common.h"

namespace OHOS {
namespace NetManagerStandard {
// Firewall policy
bool NetFirewallPolicy::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isOpen)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(inAction))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(outAction))) {
        return false;
    }
    return true;
}

sptr<NetFirewallPolicy> NetFirewallPolicy::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallPolicy> ptr = new (std::nothrow) NetFirewallPolicy();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("NetFirewallPolicy ptr is null");
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->isOpen)) {
        return nullptr;
    }
    int32_t inAction = 0;
    if (!parcel.ReadInt32(inAction)) {
        return nullptr;
    }
    int32_t outAction = 0;
    if (!parcel.ReadInt32(outAction)) {
        return nullptr;
    }
    ptr->inAction = static_cast<FirewallRuleAction>(inAction);
    ptr->outAction = static_cast<FirewallRuleAction>(outAction);
    return ptr;
}

// Pagination query input
std::string RequestParam::ToString() const
{
    std::stringstream ss;
    ss << "RequestParam:{" << NET_FIREWALL_PAGE << EQUAL << this->page << COMMA << NET_FIREWALL_PAGE_SIZE << EQUAL <<
        this->pageSize << COMMA << NET_FIREWALL_ORDER_FIELD << EQUAL << static_cast<int32_t>(this->orderField) <<
        COMMA << NET_FIREWALL_ORDER_TYPE << EQUAL << static_cast<int32_t>(this->orderType) << "}";
    return ss.str();
}

bool RequestParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(page)) {
        return false;
    }
    if (!parcel.WriteInt32(pageSize)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(orderField))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(orderType))) {
        return false;
    }
    return true;
}

sptr<RequestParam> RequestParam::Unmarshalling(Parcel &parcel)
{
    sptr<RequestParam> ptr = new (std::nothrow) RequestParam();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("RequestParam ptr is null");
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->page)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->pageSize)) {
        return nullptr;
    }
    int32_t orderField = 0;
    if (!parcel.ReadInt32(orderField)) {
        return nullptr;
    }
    ptr->orderField = static_cast<NetFirewallOrderField>(orderField);
    int32_t orderType = 0;
    if (!parcel.ReadInt32(orderType)) {
        return nullptr;
    }
    ptr->orderType = static_cast<NetFirewallOrderType>(orderType);
    return ptr;
}

// The content of the rules page
bool FirewallRulePage::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(page)) {
        return false;
    }
    if (!parcel.WriteInt32(pageSize)) {
        return false;
    }
    if (!parcel.WriteInt32(totalPage)) {
        return false;
    }
    uint32_t size = data.size();
    if (size > MAX_PAGE_SIZE) {
        return false;
    }
    if (!parcel.WriteUint32(size)) {
        return false;
    }
    for (auto value : data) {
        if (!value.Marshalling(parcel)) {
            NETMGR_EXT_LOG_E("FirewallRulePage write Marshalling to parcel failed");
            return false;
        }
    }
    return true;
}

sptr<FirewallRulePage> FirewallRulePage::Unmarshalling(Parcel &parcel)
{
    sptr<FirewallRulePage> ptr = new (std::nothrow) FirewallRulePage();
    if (ptr == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->page)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->pageSize)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->totalPage)) {
        return nullptr;
    }
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        return nullptr;
    }
    if (size > MAX_PAGE_SIZE) {
        NETMGR_EXT_LOG_E("InterceptRecordPage read list size is too large");
        return nullptr;
    }
    for (uint32_t i = 0; i < size; i++) {
        auto value = NetFirewallRule::Unmarshalling(parcel);
        if (value == nullptr) {
            NETMGR_EXT_LOG_E("FirewallRulePage read Unmarshalling to parcel failed");
            return nullptr;
        }
        ptr->data.push_back(*value);
    }
    return ptr;
}

// Intercept record pagination content
bool InterceptRecordPage::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(page)) {
        return false;
    }
    if (!parcel.WriteInt32(pageSize)) {
        return false;
    }
    if (!parcel.WriteInt32(totalPage)) {
        return false;
    }
    uint32_t size = data.size();
    if (size > MAX_PAGE_SIZE) {
        return false;
    }
    if (!parcel.WriteUint32(size)) {
        return false;
    }
    for (auto value : data) {
        if (!value.Marshalling(parcel)) {
            NETMGR_EXT_LOG_E("InterceptRecordPage write Marshalling to parcel failed");
            return false;
        }
    }
    return true;
}

sptr<InterceptRecordPage> InterceptRecordPage::Unmarshalling(Parcel &parcel)
{
    sptr<InterceptRecordPage> ptr = new (std::nothrow) InterceptRecordPage();
    if (ptr == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->page)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->pageSize)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->totalPage)) {
        return nullptr;
    }
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        return nullptr;
    }
    if (size > MAX_PAGE_SIZE) {
        NETMGR_EXT_LOG_E("InterceptRecordPage read list size is too large");
        return nullptr;
    }
    for (uint32_t i = 0; i < size; i++) {
        auto value = InterceptRecord::Unmarshalling(parcel);
        if (value == nullptr) {
            NETMGR_EXT_LOG_E("InterceptRecordPage read Unmarshalling to parcel failed");
            return nullptr;
        }
        ptr->data.push_back(*value);
    }
    return ptr;
}

// Traffic redirect related structures
bool NetTrafficFilterConfig::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(packetCopyLen)) {
        return false;
    }
    if (!parcel.WriteUint32(nfqueueMaxlen)) {
        return false;
    }
    if (!parcel.WriteUint32(nfqueueFlags)) {
        return false;
    }
    return true;
}

sptr<NetTrafficFilterConfig> NetTrafficFilterConfig::Unmarshalling(Parcel &parcel)
{
    sptr<NetTrafficFilterConfig> ptr = new (std::nothrow) NetTrafficFilterConfig();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("NetTrafficFilterConfig ptr is null");
        return nullptr;
    }
    if (!parcel.ReadUint32(ptr->packetCopyLen)) {
        return nullptr;
    }
    if (!parcel.ReadUint32(ptr->nfqueueMaxlen)) {
        return nullptr;
    }
    if (!parcel.ReadUint32(ptr->nfqueueFlags)) {
        return nullptr;
    }
    return ptr;
}

bool TrafficFilterIPAddress::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(family_)) {
        NETMGR_EXT_LOG_E("Write family failed");
        return false;
    }
    for (int i = 0; i < 16; i++) {
        if (!parcel.WriteUint8(addr_[i])) {
            NETMGR_EXT_LOG_E("Write addr byte failed at index %{public}d", i);
            return false;
        }
    }
    return true;
}

sptr<TrafficFilterIPAddress> TrafficFilterIPAddress::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterIPAddress> ptr = new (std::nothrow) TrafficFilterIPAddress();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterIPAddress failed");
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->family_)) {
        NETMGR_EXT_LOG_E("Read family failed");
        return nullptr;
    }
    for (int i = 0; i < 16; i++) {
        if (!parcel.ReadUint8(ptr->addr_[i])) {
            NETMGR_EXT_LOG_E("Read addr byte failed at index %{public}d", i);
            return nullptr;
        }
    }
    return ptr;
}

bool TrafficFilterIPCidr::Marshalling(Parcel &parcel) const
{
    if (!base_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write base IP failed");
        return false;
    }
    if (!parcel.WriteUint8(prefixLen_)) {
        NETMGR_EXT_LOG_E("Write prefixLen failed");
        return false;
    }
    return true;
}

sptr<TrafficFilterIPCidr> TrafficFilterIPCidr::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterIPCidr> ptr = new (std::nothrow) TrafficFilterIPCidr();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterIPCidr failed");
        return nullptr;
    }
    sptr<TrafficFilterIPAddress> base = TrafficFilterIPAddress::Unmarshalling(parcel);
    if (base == nullptr) {
        return nullptr;
    }
    ptr->base_ = *base;
    if (!parcel.ReadUint8(ptr->prefixLen_)) {
        NETMGR_EXT_LOG_E("Read prefixLen failed");
        return nullptr;
    }
    return ptr;
}

bool TrafficFilterIPRange::Marshalling(Parcel &parcel) const
{
    if (!start_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write start IP failed");
        return false;
    }
    if (!end_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write end IP failed");
        return false;
    }
    return true;
}

sptr<TrafficFilterIPRange> TrafficFilterIPRange::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterIPRange> ptr = new (std::nothrow) TrafficFilterIPRange();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterIPRange failed");
        return nullptr;
    }
    sptr<TrafficFilterIPAddress> start = TrafficFilterIPAddress::Unmarshalling(parcel);
    if (start == nullptr) {
        return nullptr;
    }
    ptr->start_ = *start;
    sptr<TrafficFilterIPAddress> end = TrafficFilterIPAddress::Unmarshalling(parcel);
    if (end == nullptr) {
        return nullptr;
    }
    ptr->end_ = *end;
    return ptr;
}

bool TrafficFilterIPMulti::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(ipCount_)) {
        NETMGR_EXT_LOG_E("Write ipCount failed");
        return false;
    }
    for (uint32_t i = 0; i < ipCount_; i++) {
        if (!ips_[i].Marshalling(parcel)) {
            NETMGR_EXT_LOG_E("Write IP at index %{public}u failed", i);
            return false;
        }
    }
    return true;
}

sptr<TrafficFilterIPMulti> TrafficFilterIPMulti::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterIPMulti> ptr = new (std::nothrow) TrafficFilterIPMulti();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterIPMulti failed");
        return nullptr;
    }
    if (!parcel.ReadUint32(ptr->ipCount_)) {
        NETMGR_EXT_LOG_E("Read ipCount failed");
        return nullptr;
    }
    if (ptr->ipCount_ > 16) {
        NETMGR_EXT_LOG_E("ipCount %{public}u exceeds MAX_MULTI_IP_COUNT", ptr->ipCount_);
        return nullptr;
    }
    for (uint32_t i = 0; i < ptr->ipCount_; i++) {
        sptr<TrafficFilterIPAddress> ip = TrafficFilterIPAddress::Unmarshalling(parcel);
        if (ip == nullptr) {
            return nullptr;
        }
        ptr->ips_[i] = *ip;
    }
    return ptr;
}

bool TrafficFilterIPMatch::Marshalling(Parcel &parcel) const
{
    if (!IsValidType()) {
        NETMGR_EXT_LOG_E("Write type failed: %{public}s", GetErrorInfo().c_str());
        return false;
    }
    if (!parcel.WriteInt32(type_)) {
        NETMGR_EXT_LOG_E("Write type failed");
        return false;
    }
    if (!parcel.WriteBool(invert_)) {
        NETMGR_EXT_LOG_E("Write invert failed");
        return false;
    }
    switch (type_) {
        case 0:
            break;
        case 1:
            if (!single_.Marshalling(parcel)) {
                NETMGR_EXT_LOG_E("Write single IP failed");
                return false;
            }
            break;
        case 2:
            if (!cidr_.Marshalling(parcel)) {
                NETMGR_EXT_LOG_E("Write CIDR failed");
                return false;
            }
            break;
        case 3:
            if (!range_.Marshalling(parcel)) {
                NETMGR_EXT_LOG_E("Write IP range failed");
                return false;
            }
            break;
        case 4:
            if (!multi_.Marshalling(parcel)) {
                NETMGR_EXT_LOG_E("Write IP multi failed");
                return false;
            }
            break;
        default:
            NETMGR_EXT_LOG_E("Invalid IP match type: %{public}d", type_);
            return false;
    }
    return true;
}

bool TrafficFilterIPMatch::UnmarshallingByType(Parcel &parcel)
{
    switch (type_) {
        case 0:
            return true;
        case 1: {
            sptr<TrafficFilterIPAddress> single = TrafficFilterIPAddress::Unmarshalling(parcel);
            if (single == nullptr) {
                NETMGR_EXT_LOG_E("Read single IP failed");
                return false;
            }
            single_ = *single;
            return true;
        }
        case 2: {
            sptr<TrafficFilterIPCidr> cidr = TrafficFilterIPCidr::Unmarshalling(parcel);
            if (cidr == nullptr) {
                NETMGR_EXT_LOG_E("Read CIDR failed");
                return false;
            }
            cidr_ = *cidr;
            return true;
        }
        case 3: {
            sptr<TrafficFilterIPRange> range = TrafficFilterIPRange::Unmarshalling(parcel);
            if (range == nullptr) {
                NETMGR_EXT_LOG_E("Read IP range failed");
                return false;
            }
            range_ = *range;
            return true;
        }
        case 4: {
            sptr<TrafficFilterIPMulti> multi = TrafficFilterIPMulti::Unmarshalling(parcel);
            if (multi == nullptr) {
                NETMGR_EXT_LOG_E("Read IP multi failed");
                return false;
            }
            multi_ = *multi;
            return true;
        }
        default:
            NETMGR_EXT_LOG_E("Invalid IP match type: %{public}d", type_);
            return false;
    }
}

sptr<TrafficFilterIPMatch> TrafficFilterIPMatch::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterIPMatch> ptr = new (std::nothrow) TrafficFilterIPMatch();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterIPMatch failed");
        return nullptr;
    }

    if (!parcel.ReadInt32(ptr->type_)) {
        NETMGR_EXT_LOG_E("Read type failed");
        return nullptr;
    }

    if (!ptr->IsValidType()) {
        NETMGR_EXT_LOG_E("Read invalid type: %{public}d", ptr->type_);
        return nullptr;
    }

    if (!parcel.ReadBool(ptr->invert_)) {
        NETMGR_EXT_LOG_E("Read invert failed");
        return nullptr;
    }

    if (!ptr->UnmarshallingByType(parcel)) {
        return nullptr;
    }
    return ptr;
}

bool TrafficFilterPortRange::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(startPort_)) {
        NETMGR_EXT_LOG_E("Write startPort failed");
        return false;
    }
    if (!parcel.WriteUint16(endPort_)) {
        NETMGR_EXT_LOG_E("Write endPort failed");
        return false;
    }
    return true;
}

sptr<TrafficFilterPortRange> TrafficFilterPortRange::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterPortRange> ptr = new (std::nothrow) TrafficFilterPortRange();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterPortRange failed");
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->startPort_)) {
        NETMGR_EXT_LOG_E("Read startPort failed");
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->endPort_)) {
        NETMGR_EXT_LOG_E("Read endPort failed");
        return nullptr;
    }
    return ptr;
}

bool TrafficFilterPortMulti::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(portCount_)) {
        NETMGR_EXT_LOG_E("Write portCount failed");
        return false;
    }
    for (uint32_t i = 0; i < portCount_; i++) {
        if (!parcel.WriteUint16(ports_[i])) {
            NETMGR_EXT_LOG_E("Write port at index %{public}u failed", i);
            return false;
        }
    }
    return true;
}

sptr<TrafficFilterPortMulti> TrafficFilterPortMulti::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterPortMulti> ptr = new (std::nothrow) TrafficFilterPortMulti();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterPortMulti failed");
        return nullptr;
    }
    if (!parcel.ReadUint32(ptr->portCount_)) {
        NETMGR_EXT_LOG_E("Read portCount failed");
        return nullptr;
    }
    if (ptr->portCount_ > 64) {
        NETMGR_EXT_LOG_E("portCount %{public}u exceeds MAX_MULTI_PORT_COUNT", ptr->portCount_);
        return nullptr;
    }
    for (uint32_t i = 0; i < ptr->portCount_; i++) {
        if (!parcel.ReadUint16(ptr->ports_[i])) {
            NETMGR_EXT_LOG_E("Read port at index %{public}u failed", i);
            return nullptr;
        }
    }
    return ptr;
}

bool TrafficFilterPortMatch::Marshalling(Parcel &parcel) const
{
    if (!IsValidType()) {
        NETMGR_EXT_LOG_E("Write type failed: %{public}s", GetErrorInfo().c_str());
        return false;
    }
    if (!parcel.WriteInt32(type_)) {
        NETMGR_EXT_LOG_E("Write type failed");
        return false;
    }
    if (!parcel.WriteBool(invert_)) {
        NETMGR_EXT_LOG_E("Write invert failed");
        return false;
    }
    switch (type_) {
        case 0:
            break;
        case 1:
            if (!parcel.WriteUint16(single_)) {
                NETMGR_EXT_LOG_E("Write single port failed");
                return false;
            }
            break;
        case 2:
            if (!range_.Marshalling(parcel)) {
                NETMGR_EXT_LOG_E("Write port range failed");
                return false;
            }
            break;
        case 3:
            if (!multi_.Marshalling(parcel)) {
                NETMGR_EXT_LOG_E("Write port multi failed");
                return false;
            }
            break;
        default:
            NETMGR_EXT_LOG_E("Invalid port match type: %{public}d", type_);
            return false;
    }
    return true;
}

bool TrafficFilterPortMatch::UnmarshallingByType(Parcel &parcel)
{
    switch (type_) {
        case 0:
            return true;
        case 1: {
            if (!parcel.ReadUint16(single_)) {
                NETMGR_EXT_LOG_E("Read single port failed");
                return false;
            }
            return true;
        }
        case 2: {
            sptr<TrafficFilterPortRange> range = TrafficFilterPortRange::Unmarshalling(parcel);
            if (range == nullptr) {
                NETMGR_EXT_LOG_E("Read port range failed");
                return false;
            }
            range_ = *range;
            return true;
        }
        case 3: {
            sptr<TrafficFilterPortMulti> multi = TrafficFilterPortMulti::Unmarshalling(parcel);
            if (multi == nullptr) {
                NETMGR_EXT_LOG_E("Read port multi failed");
                return false;
            }
            multi_ = *multi;
            return true;
        }
        default:
            NETMGR_EXT_LOG_E("Invalid port match type: %{public}d", type_);
            return false;
    }
}

sptr<TrafficFilterPortMatch> TrafficFilterPortMatch::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterPortMatch> ptr = new (std::nothrow) TrafficFilterPortMatch();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterPortMatch failed");
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->type_)) {
        NETMGR_EXT_LOG_E("Read type failed");
        return nullptr;
    }

    if (!ptr->IsValidType()) {
        NETMGR_EXT_LOG_E("Read invalid type: %{public}d", ptr->type_);
        return nullptr;
    }

    if (!parcel.ReadBool(ptr->invert_)) {
        NETMGR_EXT_LOG_E("Read invert failed");
        return nullptr;
    }

    if (!ptr->UnmarshallingByType(parcel)) {
        return nullptr;
    }
    return ptr;
}

bool TrafficFilterInterfaceMatch::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(enabled_)) {
        NETMGR_EXT_LOG_E("Write enabled failed");
        return false;
    }
    if (!parcel.WriteBool(invert_)) {
        NETMGR_EXT_LOG_E("Write invert failed");
        return false;
    }
    if (!parcel.WriteBool(isPrefix_)) {
        NETMGR_EXT_LOG_E("Write isPrefix failed");
        return false;
    }
    if (!parcel.WriteString(ifName_)) {
        NETMGR_EXT_LOG_E("Write ifName failed");
        return false;
    }
    return true;
}

sptr<TrafficFilterInterfaceMatch> TrafficFilterInterfaceMatch::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterInterfaceMatch> ptr = new (std::nothrow) TrafficFilterInterfaceMatch();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterInterfaceMatch failed");
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->enabled_)) {
        NETMGR_EXT_LOG_E("Read enabled failed");
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->invert_)) {
        NETMGR_EXT_LOG_E("Read invert failed");
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->isPrefix_)) {
        NETMGR_EXT_LOG_E("Read isPrefix failed");
        return nullptr;
    }
    if (!parcel.ReadString(ptr->ifName_)) {
        NETMGR_EXT_LOG_E("Read ifName failed");
        return nullptr;
    }
    return ptr;
}

bool TrafficFilterRedirectRule::MarshallingUidAndProxyFields(Parcel &parcel) const
{
    if (!parcel.WriteUint32(uidStart_)) {
        NETMGR_EXT_LOG_E("Write uidStart failed");
        return false;
    }
    if (!parcel.WriteUint32(uidEnd_)) {
        NETMGR_EXT_LOG_E("Write uidEnd failed");
        return false;
    }
    if (!proxyIp_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write proxyIp failed");
        return false;
    }
    if (!parcel.WriteUint16(proxyPort_)) {
        NETMGR_EXT_LOG_E("Write proxyPort failed");
        return false;
    }
    return true;
}

bool TrafficFilterRedirectRule::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(priority_)) {
        NETMGR_EXT_LOG_E("Write priority failed");
        return false;
    }
    if (!parcel.WriteInt32(hookPoint_)) {
        NETMGR_EXT_LOG_E("Write hookPoint failed");
        return false;
    }
    if (!parcel.WriteUint8(protocol_)) {
        NETMGR_EXT_LOG_E("Write protocol failed");
        return false;
    }
    if (!srcIp_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write srcIp failed");
        return false;
    }
    if (!srcPort_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write srcPort failed");
        return false;
    }
    if (!dstIp_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write dstIp failed");
        return false;
    }
    if (!dstPort_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write dstPort failed");
        return false;
    }
    if (!inInterface_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write inInterface failed");
        return false;
    }
    if (!outInterface_.Marshalling(parcel)) {
        NETMGR_EXT_LOG_E("Write outInterface failed");
        return false;
    }

    if (!MarshallingUidAndProxyFields(parcel)) {
        return false;
    }
    return true;
}

bool TrafficFilterRedirectRule::UnmarshalMatchAndInterfaceFields(Parcel &parcel)
{
    sptr<TrafficFilterIPMatch> srcIp = TrafficFilterIPMatch::Unmarshalling(parcel);
    if (srcIp == nullptr) {
        NETMGR_EXT_LOG_E("Read srcIp failed");
        return false;
    }
    srcIp_ = *srcIp;

    sptr<TrafficFilterPortMatch> srcPort = TrafficFilterPortMatch::Unmarshalling(parcel);
    if (srcPort == nullptr) {
        NETMGR_EXT_LOG_E("Read srcPort failed");
        return false;
    }
    srcPort_ = *srcPort;

    sptr<TrafficFilterIPMatch> dstIp = TrafficFilterIPMatch::Unmarshalling(parcel);
    if (dstIp == nullptr) {
        NETMGR_EXT_LOG_E("Read dstIp failed");
        return false;
    }
    dstIp_ = *dstIp;

    sptr<TrafficFilterPortMatch> dstPort = TrafficFilterPortMatch::Unmarshalling(parcel);
    if (dstPort == nullptr) {
        NETMGR_EXT_LOG_E("Read dstPort failed");
        return false;
    }
    dstPort_ = *dstPort;

    sptr<TrafficFilterInterfaceMatch> inInterface = TrafficFilterInterfaceMatch::Unmarshalling(parcel);
    if (inInterface == nullptr) {
        NETMGR_EXT_LOG_E("Read inInterface failed");
        return false;
    }
    inInterface_ = *inInterface;

    sptr<TrafficFilterInterfaceMatch> outInterface = TrafficFilterInterfaceMatch::Unmarshalling(parcel);
    if (outInterface == nullptr) {
        NETMGR_EXT_LOG_E("Read outInterface failed");
        return false;
    }
    outInterface_ = *outInterface;

    return true;
}

sptr<TrafficFilterRedirectRule> TrafficFilterRedirectRule::Unmarshalling(Parcel &parcel)
{
    sptr<TrafficFilterRedirectRule> ptr = new (std::nothrow) TrafficFilterRedirectRule();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("Create TrafficFilterRedirectRule failed");
        return nullptr;
    }

    if (!parcel.ReadUint32(ptr->priority_)) {
        NETMGR_EXT_LOG_E("Read priority failed");
        return nullptr;
    }

    if (!parcel.ReadInt32(ptr->hookPoint_)) {
        NETMGR_EXT_LOG_E("Read hookPoint failed");
        return nullptr;
    }

    if (!parcel.ReadUint8(ptr->protocol_)) {
        NETMGR_EXT_LOG_E("Read protocol failed");
        return nullptr;
    }

    if (!ptr->UnmarshalMatchAndInterfaceFields(parcel)) {
        return nullptr;
    }

    if (!parcel.ReadUint32(ptr->uidStart_)) {
        NETMGR_EXT_LOG_E("Read uidStart failed");
        return nullptr;
    }

    if (!parcel.ReadUint32(ptr->uidEnd_)) {
        NETMGR_EXT_LOG_E("Read uidEnd failed");
        return nullptr;
    }

    {
        sptr<TrafficFilterIPAddress> proxyIp = TrafficFilterIPAddress::Unmarshalling(parcel);
        if (proxyIp == nullptr) {
            NETMGR_EXT_LOG_E("Read proxyIp failed");
            return nullptr;
        }
        ptr->proxyIp_ = *proxyIp;
    }

    if (!parcel.ReadUint16(ptr->proxyPort_)) {
        NETMGR_EXT_LOG_E("Read proxyPort failed");
        return nullptr;
    }

    return ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS