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

#ifndef TRAFFICDESCRITORSINFO_H
#define TRAFFICDESCRITORSINFO_H

#include <string>
#include <vector>
#include <cstdint>
#include "inet_addr.h"
#include "hwosappId.h"
#include "hwnetworkslicemanager.h"

namespace OHOS {
namespace NetManagerStandard {

class TrafficDescriptorsInfo {
public:
    enum RouteBindType {
        UID_TDS,
        UID_IP_TDS,
        IP_TDS,
        INVALID_TDS
    };
    class Builder {
    public:
        Builder()
            : mUrspPrecedence(0), mAppIds(""), mIpv4Num(0), mIpv4AddrAndMask({}), mIpv6Num(0), mIpv6AddrAndPrefix({}),
            mProtocolIds(""), mRemotePorts(""), mRouteBitmap(0), mIsIpTriad(false), mIsMatchFqdn(false),
            mIsMatchDnn(false), mHasAvailableUrsp(false), mIsMatchCct(false),
            mRouteBindType(RouteBindType::INVALID_TDS), mUid(0), mDnn(""), mFqdn(""), mIp(), mProtocolId(""),
            mRemotePort(""), mFqdnIps(), mIsNeedToCreateRequest(false), mIsRequestAgain(false), mCct(0)
        {}
        Builder& setUrspPrecedence(uint8_t precedence)
        {
            mUrspPrecedence = precedence;
            return *this;
        }
        Builder& setAppIds(const std::string& ids)
        {
            mAppIds = ids;
            return *this;
        }
        Builder& setIpv4Num(uint8_t num)
        {
            mIpv4Num = num;
            return *this;
        }
        Builder& setIpv4AddrAndMask(const std::vector<uint8_t>& addrAndMask)
        {
            mIpv4AddrAndMask = addrAndMask;
            return *this;
        }
        Builder& setIpv6Num(uint8_t num)
        {
            mIpv6Num = num;
            return *this;
        }
        Builder& setIpv6AddrAndPrefix(const std::vector<uint8_t>& addrAndPrefix)
        {
            mIpv6AddrAndPrefix = addrAndPrefix;
            return *this;
        }
        Builder& setProtocolIds(const std::string& ids)
        {
            mProtocolIds = ids;
            return *this;
        }
        Builder& setRemotePorts(const std::string& ports)
        {
            mRemotePorts = ports;
            return *this;
        }
        Builder& setRouteBitmap(uint8_t bitmap)
        {
            mRouteBitmap = bitmap;
            return *this;
        }
        Builder& setUid(int uid)
        {
            mUid = uid;
            return *this;
        }
        Builder& setDnn(const std::string& dnn)
        {
            mDnn = dnn;
            return *this;
        }
        Builder& setCct(int cct)
        {
            mCct = cct;
            return *this;
        }
        Builder& setFqdn(const std::string& fqdn)
        {
            mFqdn = fqdn;
            return *this;
        }
        Builder& setIp(const INetAddr& ip)
        {
            mIp = ip;
            return *this;
        }
        Builder& setProtocolId(const std::string& id)
        {
            mProtocolId = id;
            return *this;
        }
        Builder& setRemotePort(const std::string& port)
        {
            mRemotePort = port;
            return *this;
        }
        Builder& setFqdnIps(const FqdnIps& fqdnIps)
        {
            mFqdnIps = fqdnIps;
            return *this;
        }
        Builder& setNeedToCreateRequest(bool needToCreateRequest)
        {
            mIsNeedToCreateRequest = needToCreateRequest;
            return *this;
        }
        TrafficDescriptorsInfo build()
        {
            TrafficDescriptorsInfo trafficDescriptors;
            trafficDescriptors.mUrspPrecedence = mUrspPrecedence;
            trafficDescriptors.mAppIds = trafficDescriptors.getUnsignedAppIds(mAppIds);
            trafficDescriptors.mIpv4Num = mIpv4Num;
            trafficDescriptors.mIpv4AddrAndMask = mIpv4AddrAndMask;
            trafficDescriptors.mIpv6Num = mIpv6Num;
            trafficDescriptors.mIpv6AddrAndPrefix = mIpv6AddrAndPrefix;
            trafficDescriptors.mProtocolIds = mProtocolIds;
            trafficDescriptors.mRemotePorts = mRemotePorts;
            trafficDescriptors.mRouteBitmap = mRouteBitmap;
            trafficDescriptors.mUid = mUid;
            trafficDescriptors.mDnn = mDnn;
            trafficDescriptors.mFqdn = mFqdn;
            trafficDescriptors.mIp = mIp;
            trafficDescriptors.mProtocolId = mProtocolId;
            trafficDescriptors.mRemotePort = mRemotePort;
            trafficDescriptors.mFqdnIps = mFqdnIps;
            trafficDescriptors.mCct = mCct;
            trafficDescriptors.mIsNeedToCreateRequest = mIsNeedToCreateRequest;
            trafficDescriptors.mIsIpTriad = (mIpv4Num != 0) || (mIpv6Num != 0);
            trafficDescriptors.mIsMatchFqdn = (mRouteBitmap & MATCH_FQDN) != 0;
            trafficDescriptors.mIsMatchDnn = (mRouteBitmap & MATCH_DNN) != 0;
            trafficDescriptors.mHasAvailableUrsp = (mRouteBitmap & MATCH_AVAILABLE) != 0;
            trafficDescriptors.mIsMatchCct = (mRouteBitmap & MATCH_CCT) != 0;
            bool hasIps = mIsIpTriad || mIsMatchFqdn;
            bool hasAppids = !mAppIds.empty() || mIsMatchDnn;
            mRouteBindType = TrafficDescriptorsInfo::RouteBindType::INVALID_TDS;
            if (hasAppids && hasIps) {
                mRouteBindType = TrafficDescriptorsInfo::RouteBindType::UID_IP_TDS;
            } else if (hasAppids) {
                mRouteBindType = TrafficDescriptorsInfo::RouteBindType::UID_TDS;
            } else if (hasIps) {
                mRouteBindType = TrafficDescriptorsInfo::RouteBindType::IP_TDS;
            }
            trafficDescriptors.mRouteBindType = mRouteBindType;
            return trafficDescriptors;
        }
    private:
        uint8_t mUrspPrecedence;
        std::string mAppIds;
        uint8_t mIpv4Num;
        std::vector<uint8_t> mIpv4AddrAndMask;
        uint8_t mIpv6Num;
        std::vector<uint8_t> mIpv6AddrAndPrefix;
        std::string mProtocolIds;
        std::string mRemotePorts;
        uint8_t mRouteBitmap;
        bool mIsIpTriad;
        bool mIsMatchFqdn;
        bool mIsMatchDnn;
        bool mHasAvailableUrsp;
        bool mIsMatchCct;
        TrafficDescriptorsInfo::RouteBindType mRouteBindType;
        int mUid;
        std::string mDnn;
        std::string mFqdn;
        INetAddr mIp;
        std::string mProtocolId;
        std::string mRemotePort;
        FqdnIps mFqdnIps;
        bool mIsNeedToCreateRequest;
        bool mIsRequestAgain;
        int mCct;
    };
    static const int CCT_TYPE_INVALID;
    static const int CCT_TYPE_IMS;
    static const int CCT_TYPE_MMS;
    static const int CCT_TYPE_SUPL;
    static const std::string TDS_ROUTE_BITMAP;
    static const std::string TDS_URSP_PRECEDENCE;
    static const std::string TDS_APPIDS;
    static const std::string TDS_IPV4_NUM;
    static const std::string TDS_IPV4_ADDRANDMASK;
    static const std::string TDS_IPV6_NUM;
    static const std::string TDS_IPV6_ADDRANDPREFIX;
    static const std::string TDS_PROTOCOLIDS;
    static const std::string TDS_REMOTEPORTS;
    static const std::string SEPARATOR;
    static const int ROUTE_BITMAP_BIT4;
    static const uint8_t MATCH_DNN;
    static const uint8_t MATCH_FQDN;
    static const uint8_t MATCH_AVAILABLE;
    static const uint8_t MATCH_CCT;
    TrafficDescriptorsInfo()
        : mUrspPrecedence(0), mAppIds(""), mIpv4Num(0), mIpv4AddrAndMask({}), mIpv6Num(0), mIpv6AddrAndPrefix({}),
        mProtocolIds(""), mRemotePorts(""), mRouteBitmap(0), mIsIpTriad(false), mIsMatchFqdn(false), mIsMatchDnn(false),
        mHasAvailableUrsp(false), mIsMatchCct(false), mRouteBindType(RouteBindType::INVALID_TDS), mUid(0), mDnn(""),
        mFqdn(""), mIp(), mProtocolId(""), mRemotePort(""), mFqdnIps(), mIsNeedToCreateRequest(false),
        mIsRequestAgain(false), mCct(0) {}

    static TrafficDescriptorsInfo makeTrafficDescriptorsInfo(std::map<std::string, std::string>& data)
    {
        std::string appIds = "";
        if (data.find(TDS_APPIDS) != data.end()) {
            appIds = data[TDS_APPIDS];
        }
        uint8_t urspPrecedence = 0;
        if (data.find(TDS_URSP_PRECEDENCE) != data.end()) {
            urspPrecedence = static_cast<uint8_t>(std::stoi(data[TDS_URSP_PRECEDENCE]));
        }
        uint8_t ipv4Num = 0;
        if (data.find(TDS_IPV4_NUM) != data.end()) {
            ipv4Num = static_cast<uint8_t>(std::stoi(data[TDS_IPV4_NUM]));
        }
        uint8_t ipv6Num = 0;
        if (data.find(TDS_IPV6_NUM) != data.end()) {
            ipv6Num = static_cast<uint8_t>(std::stoi(data[TDS_IPV6_NUM]));
        }
        std::string ipv4AddrAndMaskStr = "";
        if (data.find(TDS_IPV4_ADDRANDMASK) != data.end()) {
            ipv4AddrAndMaskStr = data[TDS_IPV4_ADDRANDMASK];
        }
        std::string ipv6AddrAndPrefixStr = "";
        if (data.find(TDS_IPV6_ADDRANDPREFIX) != data.end()) {
            ipv6AddrAndPrefixStr = data[TDS_IPV6_ADDRANDPREFIX];
        }
        std::string protocolIds = "";
        if (data.find(TDS_PROTOCOLIDS) != data.end()) {
            protocolIds = data[TDS_PROTOCOLIDS];
        }
        std::string remotePorts = "";
        if (data.find(TDS_REMOTEPORTS) != data.end()) {
            remotePorts = data[TDS_REMOTEPORTS];
        }
        uint8_t routeBitmap = 0;
        if (data.find(TDS_ROUTE_BITMAP) != data.end()) {
            routeBitmap = static_cast<uint8_t>(std::stoi(data[TDS_ROUTE_BITMAP]));
        }
        TrafficDescriptorsInfo::Builder builder;
        builder.setAppIds(appIds);
        builder.setUrspPrecedence(urspPrecedence);
        builder.setIpv4Num(ipv4Num);
        builder.setIpv4AddrAndMask(ConvertstringTouInt8Vector(ipv4AddrAndMaskStr));
        builder.setIpv6Num(ipv6Num);
        builder.setIpv6AddrAndPrefix(ConvertstringTouInt8Vector(ipv6AddrAndPrefixStr));
        builder.setProtocolIds(protocolIds);
        builder.setRemotePorts(remotePorts);
        builder.setRouteBitmap(routeBitmap);
        return builder.build();
    }

    std::string getUnsignedAppIds(const std::string& appIds)
    {
        if (appIds.empty()) {
            return "";
        }
        std::vector<std::string> osAppIds;
        osAppIds = Split(appIds, SEPARATOR);
        for (int i = 0; i < (int)osAppIds.size(); ++i) {
            HwOsAppId osAppId = HwOsAppId::Create(osAppIds[i]);
            if (osAppId.getOsId().empty() && osAppId.getAppId().empty()) {
                continue;
            }
        }
        std::string result;
        for (const auto& id : osAppIds) {
            if (!id.empty()) {
                result += id + SEPARATOR;
            }
        }
        return result;
    }
    bool isUidRouteBindType() const
    {
        return mRouteBindType == RouteBindType::UID_IP_TDS || mRouteBindType == RouteBindType::UID_TDS;
    }
    bool isMatchDnn() const
    {
        return mIsMatchDnn;
    }
    bool isMatchNetworkCap() const
    {
        return mIsMatchDnn || mIsMatchCct;
    }
    bool isMatchFqdn() const
    {
        return mIsMatchFqdn;
    }
    bool hasAvailableUrsp() const
    {
        return mHasAvailableUrsp;
    }
    bool isAtiveTriggeringApp(const std::string& packageName) const
    {
        return (std::find(mAtiveTriggeringApps.begin(), mAtiveTriggeringApps.end(), packageName)
            != mAtiveTriggeringApps.end());
    }
    void setRequestAgain(bool requestAgain)
    {
        mIsRequestAgain = requestAgain;
    }
    bool isRequestAgain()
    {
        return mIsRequestAgain;
    }
    RouteBindType getRouteBindType() const
    {
        return mRouteBindType;
    }
    uint8_t getUrspPrecedence() const
    {
        return mUrspPrecedence;
    }
    const std::string& getAppIds() const
    {
        return mAppIds;
    }
    uint8_t getIpv4Num() const
    {
        return mIpv4Num;
    }
    const std::vector<uint8_t>& getIpv4AddrAndMask() const
    {
        return mIpv4AddrAndMask;
    }
    uint8_t getIpv6Num() const
    {
        return mIpv6Num;
    }
    const std::vector<uint8_t>& getIpv6AddrAndPrefix() const
    {
        return mIpv6AddrAndPrefix;
    }
    const std::string& getProtocolIds() const
    {
        return mProtocolIds;
    }
    const std::string& getRemotePorts() const
    {
        return mRemotePorts;
    }
    uint8_t getRouteBitmap() const
    {
        return mRouteBitmap;
    }
    const FqdnIps& getFqdnIps() const
    {
        return mFqdnIps;
    }
    int getUid() const
    {
        return mUid;
    }
    const std::string& getDnn() const
    {
        return mDnn;
    }
    int getCct() const
    {
        return mCct;
    }
    const std::string& getFqdn() const
    {
        return mFqdn;
    }
    const INetAddr& getIp() const
    {
        return mIp;
    }
    const std::string& getProtocolId() const
    {
        return mProtocolId;
    }
    const std::string& getRemotePort() const
    {
        return mRemotePort;
    }
    bool isNeedToCreateRequest() const
    {
        return mIsNeedToCreateRequest;
    }
    bool isIpTriad() const
    {
        return mIsIpTriad;
    }
    bool operator==(const TrafficDescriptorsInfo& other) const
    {
        return mRouteBitmap == other.mRouteBitmap && mUrspPrecedence == other.mUrspPrecedence
        && mAppIds == other.mAppIds && mIpv4Num == other.mIpv4Num && mIpv6Num == other.mIpv6Num
        && mIsIpTriad == other.mIsIpTriad && mIsMatchDnn == other.mIsMatchDnn && mIsMatchFqdn == other.mIsMatchFqdn
        && mHasAvailableUrsp == other.mHasAvailableUrsp && mIsMatchCct == other.mIsMatchCct && mUid == other.mUid
        && mIsNeedToCreateRequest == other.mIsNeedToCreateRequest && mIsRequestAgain == other.mIsRequestAgain
        && mCct == other.mCct && mIpv4AddrAndMask == other.mIpv4AddrAndMask
        && mIpv6AddrAndPrefix == other.mIpv6AddrAndPrefix && mProtocolIds == other.mProtocolIds
        && mRemotePorts == other.mRemotePorts && mAtiveTriggeringApps == other.mAtiveTriggeringApps
        && mRouteBindType == other.mRouteBindType && mDnn == other.mDnn && mFqdn == other.mFqdn
        && mIp == other.getIp() && mProtocolId == other.mProtocolId
        && mRemotePort == other.mRemotePort && mFqdnIps == other.mFqdnIps;
    }
    bool operator<(const TrafficDescriptorsInfo& other) const
    {
        if (std::tie(mRouteBitmap, mUrspPrecedence, mAppIds, mIpv4Num, mIpv6Num, mIsIpTriad, mIsMatchDnn, mIsMatchFqdn,
            mHasAvailableUrsp, mIsMatchCct, mUid, mIsNeedToCreateRequest, mIsRequestAgain, mCct, mIpv4AddrAndMask,
            mIpv6AddrAndPrefix, mProtocolIds, mRemotePorts, mAtiveTriggeringApps, mRouteBindType, mDnn, mFqdn, mIp,
            mProtocolId, mRemotePort, mFqdnIps) < std::tie(other.mRouteBitmap, other.mUrspPrecedence, other.mAppIds,
            other.mIpv4Num, other.mIpv6Num, other.mIsIpTriad, other.mIsMatchDnn, other.mIsMatchFqdn,
            other.mHasAvailableUrsp, other.mIsMatchCct, other.mUid, other.mIsNeedToCreateRequest, other.mIsRequestAgain,
            other.mCct, other.mIpv4AddrAndMask, other.mIpv6AddrAndPrefix, other.mProtocolIds, other.mRemotePorts,
            other.mAtiveTriggeringApps, other.mRouteBindType, other.mDnn, other.mFqdn, other.getIp(), other.mProtocolId,
            other.mRemotePort, other.mFqdnIps)) {
            return true;
        }
        return false;
    }
private:
    uint8_t mUrspPrecedence;
    std::string mAppIds;
    uint8_t mIpv4Num;
    std::vector<uint8_t> mIpv4AddrAndMask;
    uint8_t mIpv6Num;
    std::vector<uint8_t> mIpv6AddrAndPrefix;
    std::string mProtocolIds;
    std::string mRemotePorts;
    uint8_t mRouteBitmap;
    bool mIsIpTriad;
    bool mIsMatchFqdn;
    bool mIsMatchDnn;
    bool mHasAvailableUrsp;
    bool mIsMatchCct;
    RouteBindType mRouteBindType;
    int mUid;
    std::string mDnn;
    std::string mFqdn;
    INetAddr mIp;
    std::string mProtocolId;
    std::string mRemotePort;
    FqdnIps mFqdnIps;
    bool mIsNeedToCreateRequest;
    bool mIsRequestAgain;
    int mCct;
    std::vector<std::string> mAtiveTriggeringApps;
};

const inline int TrafficDescriptorsInfo::CCT_TYPE_INVALID = -1;
const inline int TrafficDescriptorsInfo::CCT_TYPE_IMS = 1;
const inline int TrafficDescriptorsInfo::CCT_TYPE_MMS = 2;
const inline int TrafficDescriptorsInfo::CCT_TYPE_SUPL = 4;
const inline std::string TrafficDescriptorsInfo::TDS_ROUTE_BITMAP = "routeBitmap";
const inline std::string TrafficDescriptorsInfo::TDS_URSP_PRECEDENCE = "urspPrecedence";
const inline std::string TrafficDescriptorsInfo::TDS_APPIDS = "appIds";
const inline std::string TrafficDescriptorsInfo::TDS_IPV4_NUM = "ipv4Num";
const inline std::string TrafficDescriptorsInfo::TDS_IPV4_ADDRANDMASK = "ipv4AddrAndMask";
const inline std::string TrafficDescriptorsInfo::TDS_IPV6_NUM = "ipv6Num";
const inline std::string TrafficDescriptorsInfo::TDS_IPV6_ADDRANDPREFIX = "ipv6AddrAndPrefix";
const inline std::string TrafficDescriptorsInfo::TDS_PROTOCOLIDS = "protocolIds";
const inline std::string TrafficDescriptorsInfo::TDS_REMOTEPORTS = "remotePorts";
const inline std::string TrafficDescriptorsInfo::SEPARATOR = ",";
const inline int TrafficDescriptorsInfo::ROUTE_BITMAP_BIT4 = 4;
const inline uint8_t TrafficDescriptorsInfo::MATCH_DNN = (uint8_t) (1 << 1);
const inline uint8_t TrafficDescriptorsInfo::MATCH_FQDN = (uint8_t) (1 << 2);
const inline uint8_t TrafficDescriptorsInfo::MATCH_AVAILABLE = (uint8_t) (1 << 3);
const inline uint8_t TrafficDescriptorsInfo::MATCH_CCT = (uint8_t) (1 << TrafficDescriptorsInfo::ROUTE_BITMAP_BIT4);
} // namespace NetManagerStandard
} // namespace OHOS

#endif  // ROUTESELECTIONDESCRIPTORINFO_H
