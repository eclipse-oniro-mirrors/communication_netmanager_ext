/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef NETWORKSLICECOMMCONFIG_H
#define NETWORKSLICECOMMCONFIG_H

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkSliceCommConfig {
public:
    /** move bit 8 */
    static constexpr int BIT_MOVE_8 = 8;
    /** move bit 16 */
    static constexpr int BIT_MOVE_16 = 16;
    static constexpr int LEN_BYTE = 1;
    static constexpr int LEN_SHORT = 2;
    static constexpr int LEN_THREE_BYTE = 3;
    static constexpr int LEN_INT = 4;
    static constexpr int LEN_SIX_BYTE = 6;
    static constexpr int LEN_EIGHT_BYTE = 8;
    static constexpr int LEN_SIXTEEN_BYTE = 16;
    static constexpr int HASH_MAP_DEFAULT_CAPACITY = 16;
    /** ursp version 1510 */
    static constexpr int URSP_VERSION_1510 = 1510;
    /** ursp version 1520 */
    static constexpr int URSP_VERSION_1520 = 1520;
    static constexpr int LEN_IPV6ADDR = 16;
};


class OsAppId {
public:
    void setOsId(std::string osId)
    {
        mOsId = osId;
    }

    const std::string& getOsId() const
    {
        return mOsId;
    }

    void setAppId(std::string appId)
    {
        mAppId = appId;
    }

    const std::string& getAppId() const
    {
        return mAppId;
    }
    nlohmann::json to_json() const
    {
        return nlohmann::json::object({
            {"OsId", mOsId},
            {"AppId", mAppId}
        });
    }
    void from_json(const nlohmann::json& jsonAppId)
    {
        if (jsonAppId.find("OsId") != jsonAppId.end()) {
            this->mOsId = jsonAppId.at("OsId").get<std::string>();
            NETMGR_EXT_LOG_I("OsAppId find OsId = %{public}s", this->mOsId.c_str());
        }
        if (jsonAppId.find("AppId") != jsonAppId.end()) {
            this->mAppId = jsonAppId.at("AppId").get<std::string>();
            NETMGR_EXT_LOG_I("OsAppId find AppId = %{public}s", this->mAppId.c_str());
        }
    }
private:
    std::string mOsId;
    std::string mAppId;
};
 
class Ipv4Addr {
public:
    void setIpv4Addr(const uint32_t ipv4Addr)
    {
        mIpv4Addr = ipv4Addr;
    }

    const uint32_t& getIpv4Addr() const
    {
        return mIpv4Addr;
    }

    void setIpv4Mask(const uint32_t ipv4Mask)
    {
        mIpv4Mask = ipv4Mask;
    }

    const uint32_t& getIpv4Mask() const
    {
        return mIpv4Mask;
    }

    nlohmann::json to_json() const
    {
        return nlohmann::json::object({
            {"Ipv4Addr", mIpv4Addr},
            {"Ipv4Mask", mIpv4Mask}
        });
    }

    void from_json(const nlohmann::json& jsonAddr)
    {
        if (jsonAddr.find("ipv4Addr") != jsonAddr.end()) {
            this->mIpv4Addr = jsonAddr.at("ipv4Addr").get<uint32_t>();
        }
        if (jsonAddr.find("ipv4Mask") != jsonAddr.end()) {
            this->mIpv4Mask = jsonAddr.at("ipv4Mask").get<uint32_t>();
        }
    }
private:
    uint32_t mIpv4Addr;
    uint32_t mIpv4Mask;
};

class Ipv6Addr {
public:
    void setIpv6Addr(const std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR>& ipv6Addr)
    {
        mIpv6Addr = ipv6Addr;
    }
    const std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR>& getIpv6Addr() const
    {
        return mIpv6Addr;
    }
    void setIpv6PrefixLen(int ipv6PrefixLen)
    {
        mIpv6PrefixLen = ipv6PrefixLen;
    }
    int getIpv6PrefixLen() const
    {
        return mIpv6PrefixLen;
    }
    nlohmann::json to_json() const
    {
        return nlohmann::json::object({
            {"Ipv6Addr", mIpv6Addr},
            {"Ipv6PrefixLen", mIpv6PrefixLen}
        });
    }
    void from_json(const nlohmann::json& jsonAddr)
    {
        if (jsonAddr.find("ipv6Addr") != jsonAddr.end()) {
            const nlohmann::json& ipv6AddrJson = jsonAddr.at("ipv6Addr");
            this->mIpv6Addr.fill(0);
            for (size_t i = 0; i < ipv6AddrJson.size(); ++i) {
                this->mIpv6Addr[i] = ipv6AddrJson[i].get<uint8_t>();
            }
        }
        if (jsonAddr.find("ipv6PrefixLen") != jsonAddr.end()) {
            this->mIpv6PrefixLen = jsonAddr.at("ipv6PrefixLen").get<int>();
        }
    }
private:
    std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR> mIpv6Addr;
    int mIpv6PrefixLen;
};
class RemotePortRange {
public:
    void setPortRangeLowLimit(int portRangeLowLimit)
    {
        mPortRangeLowLimit = portRangeLowLimit;
    }

    int getPortRangeLowLimit() const
    {
        return mPortRangeLowLimit;
    }

    void setPortRangeHighLimit(int portRangeHighLimit)
    {
        mPortRangeHighLimit = portRangeHighLimit;
    }

    int getPortRangeHighLimit() const
    {
        return mPortRangeHighLimit;
    }
    nlohmann::json to_json() const
    {
        return nlohmann::json::object({
            {"PortRangeLowLimit", mPortRangeLowLimit},
            {"PortRangeHighLimit", mPortRangeHighLimit}
        });
    }
    void from_json(const nlohmann::json& jsonRange)
    {
        if (jsonRange.find("portRangeLowLimit") != jsonRange.end()) {
            this->mPortRangeLowLimit = jsonRange.at("portRangeLowLimit").get<int>();
        }
        if (jsonRange.find("portRangeHighLimit") != jsonRange.end()) {
            this->mPortRangeHighLimit = jsonRange.at("portRangeHighLimit").get<int>();
        }
    }
private:
    int mPortRangeLowLimit;
    int mPortRangeHighLimit;
};

class GetSlicePara {
public:
    bool isDone = false;
    std::map<std::string, std::string> data;
    std::map<std::string, std::string> ret;
};

class AppDescriptor {
public:
    int mUid = 0;
    OsAppId mOsAppId;
    uint32_t mIpv4Addr;
    std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR> mIpv6Addr;
    int mProtocolId = 0;
    int mRemotePort = 0;
    std::string mDnn = "";
    std::string mFqdn = "";
    int mConnectionCapability = 0;

    void setUid(int uid)
    {
        mUid = uid;
    }
    int getUid()
    {
        return mUid;
    }
    void setOsAppId(const std::string& osId, const std::string& appId)
    {
        mOsAppId.setOsId(osId);
        mOsAppId.setAppId(appId);
    }
    const OsAppId& getOsAppId() const
    {
        return mOsAppId;
    }
    void setIpv4Addr(const uint32_t ipv4Addr)
    {
        mIpv4Addr = ipv4Addr;
    }
    const uint32_t& getIpv4Addr() const
    {
        return mIpv4Addr;
    }
    void setIpv6Addr(const std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR>& ipv6Addr)
    {
        mIpv6Addr = ipv6Addr;
    }
    const std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR>& getIpv6Addr() const
    {
        return mIpv6Addr;
    }
    void setProtocolId(int protocolId)
    {
        mProtocolId = protocolId;
    }
    int getProtocolId() const
    {
        return mProtocolId;
    }
    void setRemotePort(int remotePort)
    {
        mRemotePort = remotePort;
    }
    int getRemotePort() const
    {
        return mRemotePort;
    }
    void setDnn(const std::string& dnn)
    {
        mDnn = dnn;
    }
    const std::string& getDnn() const
    {
        return mDnn;
    }
    void setFqdn(const std::string& fqdn)
    {
        mFqdn = fqdn;
    }
    const std::string& getFqdn() const
    {
        return mFqdn;
    }
    void setConnectionCapability(int connCap)
    {
        mConnectionCapability = connCap;
    }
    int getConnectionCapability() const
    {
        return mConnectionCapability;
    }
};

class SelectedRouteDescriptor {
public:
    uint8_t mSscMode = 0;
    int mPduSessionType = -1;
    std::string mSnssai = "";
    std::string mDnn = "";

    /* bit0: isMatchAll bit1: isDnnMatched bit2: isFqdnMatched bit3: hasUrsp bit4: isCctMatched */
    uint8_t mRouteBitmap = 0;
    std::string mAppIds = "";
    uint8_t mIpv4Num = 0;
    std::vector<uint8_t> mIpv4AddrAndMask;
    uint8_t mIpv6Num = 0;
    std::vector<uint8_t> mIpv6AddrAndPrefix;
    std::string mProtocolIds = "";
    std::string mRemotePorts = "";
    uint8_t mUrspPrecedence = 0;

    void setSscMode(uint8_t sscMode)
    {
        mSscMode = sscMode;
    }

    uint8_t getSscMode() const
    {
        return mSscMode;
    }

    void setPduSessionType(int pduSessionType)
    {
        mPduSessionType = pduSessionType;
    }

    int getPduSessionType() const
    {
        return mPduSessionType;
    }

    void setSnssai(const std::string& snssai)
    {
        mSnssai = snssai;
    }

    const std::string& getSnssai() const
    {
        return mSnssai;
    }

    void setDnn(const std::string& dnn)
    {
        mDnn = dnn;
    }

    const std::string& getDnn() const
    {
        return mDnn;
    }

    void setRouteBitmap(uint8_t routeBitmap)
    {
        mRouteBitmap = routeBitmap;
    }

    uint8_t getRouteBitmap() const
    {
        return mRouteBitmap;
    }

    void setAppIds(const std::string& appIds)
    {
        mAppIds = appIds;
    }

    const std::string& getAppIds() const
    {
        return mAppIds;
    }

    void setIpv4Num(uint8_t ipv4Num)
    {
        mIpv4Num = ipv4Num;
    }

    uint8_t getIpv4Num() const
    {
        return mIpv4Num;
    }

    void setIpv4AddrAndMask(const std::vector<uint8_t>& ipv4AddrAndMask)
    {
        mIpv4AddrAndMask = ipv4AddrAndMask;
    }

    const std::vector<uint8_t>& getIpv4AddrAndMask() const
    {
        return mIpv4AddrAndMask;
    }

    void setIpv6Num(uint8_t ipv6Num)
    {
        mIpv6Num = ipv6Num;
    }

    uint8_t getIpv6Num() const
    {
        return mIpv6Num;
    }

    void setIpv6AddrAndPrefix(const std::vector<uint8_t>& ipv6AddrAndPrefix)
    {
        mIpv6AddrAndPrefix = ipv6AddrAndPrefix;
    }

    const std::vector<uint8_t>& getIpv6AddrAndPrefix() const
    {
        return mIpv6AddrAndPrefix;
    }

    void setProtocolIds(const std::string& protocolIds)
    {
        mProtocolIds = protocolIds;
    }

    const std::string& getProtocolIds() const
    {
        return mProtocolIds;
    }

    void setRemotePorts(const std::string& remotePorts)
    {
        mRemotePorts = remotePorts;
    }

    const std::string& getRemotePorts() const
    {
        return mRemotePorts;
    }

    void setUrspPrecedence(uint8_t urspPrecedence)
    {
        mUrspPrecedence = urspPrecedence;
    }

    uint8_t getUrspPrecedence() const
    {
        return mUrspPrecedence;
    }
};

class ForbiddenRouteDescriptor {
public:
    ForbiddenRouteDescriptor() : mSscMode(0), mPduSessionType(-1), mSnssai(""), mDnn("") {}

    void setSscMode(unsigned char sscMode)
    {
        mSscMode = sscMode;
    }

    unsigned char getSscMode() const
    {
        return mSscMode;
    }

    void setPduSessionType(int pduSessionType)
    {
        mPduSessionType = pduSessionType;
    }

    int getPduSessionType() const
    {
        return mPduSessionType;
    }

    void setSnssai(const std::string& snssai)
    {
        mSnssai = snssai;
    }

    const std::string& getSnssai() const
    {
        return mSnssai;
    }

    void setDnn(const std::string& dnn)
    {
        mDnn = dnn;
    }

    const std::string& getDnn() const
    {
        return mDnn;
    }

    void setCurrentTimeMillies(long long currentTimeMillies)
    {
        mCurrentTimeMillies = currentTimeMillies;
    }

    int64_t getCurrentTimeMillies() const
    {
        return mCurrentTimeMillies;
    }

private:
    unsigned char mSscMode;
    int mPduSessionType;
    std::string mSnssai;
    std::string mDnn;
    int64_t mCurrentTimeMillies;
};

class AddRoutePara {
public:
    int netId = 0;
    int uidNum = 0;
    int protocolIdNum = 0;
    int remotePortNum = 0;
    int remotePortRangeNum = 0;
    int singleRemotePortNum = 0;
    short len = 0;
    uint8_t urspPrecedence = 0;
    uint8_t ipv4Num = 0;
    uint8_t ipv6Num = 0;
    std::vector<int> uidArrays;
    std::vector<std::string> protocolIdArrays;
    std::vector<std::string> remotePortsArrays;
    std::vector<uint8_t> ipv4AddrAndMasks;
    std::vector<uint8_t> ipv6AddrAndPrefixs;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif  // NETWORKSLICECOMMCONFIG_H
