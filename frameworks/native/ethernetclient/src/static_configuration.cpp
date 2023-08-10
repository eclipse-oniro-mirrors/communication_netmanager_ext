/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "static_configuration.h"

#include "inet_addr.h"
#include "netmanager_base_common_utils.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_DNS_SIZE = 10;
constexpr uint32_t MAX_ADDR_SIZE = 2;

constexpr const char *SEPARATOR = ",";
} // namespace

bool StaticConfiguration::Marshalling(Parcel &parcel) const
{
    return MarshallingNetAddressList(ipAddrList_, MAX_ADDR_SIZE, parcel) &&
           MarshallingNetAddressList(routeList_, MAX_ADDR_SIZE, parcel) &&
           MarshallingNetAddressList(gatewayList_, MAX_ADDR_SIZE, parcel) &&
           MarshallingNetAddressList(netMaskList_, MAX_ADDR_SIZE, parcel) &&
           MarshallingNetAddressList(dnsServers_, MAX_DNS_SIZE, parcel) && parcel.WriteString(domain_);
}

bool StaticConfiguration::MarshallingNetAddressList(const std::vector<INetAddr> &netAddrList, uint32_t maxSize,
                                                    Parcel &parcel) const
{
    uint32_t size = static_cast<uint32_t>(std::min(maxSize, static_cast<uint32_t>(netAddrList.size())));
    if (!parcel.WriteUint32(size)) {
        NETMGR_EXT_LOG_E("write netAddrList size to parcel failed");
        return false;
    }

    for (uint32_t index = 0; index < size; ++index) {
        auto netAddr = netAddrList[index];
        if (!netAddr.Marshalling(parcel)) {
            NETMGR_EXT_LOG_E("write INetAddr to parcel failed");
            return false;
        }
    }
    return true;
}

sptr<StaticConfiguration> StaticConfiguration::Unmarshalling(Parcel &parcel)
{
    sptr<StaticConfiguration> ptr = new (std::nothrow) StaticConfiguration();
    if (ptr == nullptr) {
        NETMGR_EXT_LOG_E("ptr new failed");
        return nullptr;
    }

    bool ret = UnmarshallingNetAddressList(parcel, ptr->ipAddrList_, MAX_ADDR_SIZE) &&
               UnmarshallingNetAddressList(parcel, ptr->routeList_, MAX_ADDR_SIZE) &&
               UnmarshallingNetAddressList(parcel, ptr->gatewayList_, MAX_ADDR_SIZE) &&
               UnmarshallingNetAddressList(parcel, ptr->netMaskList_, MAX_ADDR_SIZE) &&
               UnmarshallingNetAddressList(parcel, ptr->dnsServers_, MAX_DNS_SIZE) && parcel.ReadString(ptr->domain_);
    return ret ? ptr : nullptr;
}

bool StaticConfiguration::UnmarshallingNetAddressList(Parcel &parcel, std::vector<INetAddr> &netAddrList,
                                                      uint32_t maxSize)
{
    std::vector<INetAddr>().swap(netAddrList);

    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        NETMGR_EXT_LOG_E("Read INetAddr list size failed");
        return false;
    }
    size = (size > maxSize) ? maxSize : size;
    for (uint32_t i = 0; i < size; i++) {
        auto netAddr = INetAddr::Unmarshalling(parcel);
        if (netAddr == nullptr) {
            return false;
        }
        netAddrList.push_back(*netAddr);
    }
    return true;
}

void StaticConfiguration::ExtractNetAddrBySeparator(const std::string &input, std::vector<INetAddr> &netAddrList)
{
    std::vector<INetAddr>().swap(netAddrList);
    for (const auto &netAddr : CommonUtils::Split(input, SEPARATOR)) {
        INetAddr addr;
        addr.address_ = netAddr;
        netAddrList.push_back(addr);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS