/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
// Firewall status
bool NetFirewallStatus::Marshalling(Parcel &parcel) const
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

sptr<NetFirewallStatus> NetFirewallStatus::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallStatus> ptr = new (std::nothrow) NetFirewallStatus();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("NetFirewallStatus ptr is null");
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
        this->pageSize << COMMA << NET_FIREWALL_ORDER_FILED << EQUAL << static_cast<int32_t>(this->orderFiled) <<
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
    if (!parcel.WriteInt32(static_cast<int32_t>(orderFiled))) {
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
    int32_t orderFiled = 0;
    if (!parcel.ReadInt32(orderFiled)) {
        return nullptr;
    }
    ptr->orderFiled = static_cast<NetFirewallOrderFiled>(orderFiled);
    int32_t orderType = 0;
    if (!parcel.ReadInt32(orderType)) {
        return nullptr;
    }
    ptr->orderType = static_cast<NetFirewallOrderType>(orderType);
    return ptr;
}

// 规则页面内容
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
} // namespace NetManagerStandard
} // namespace OHOS