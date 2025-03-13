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

#ifndef ROUTESELECTIONDESCRIPTORINFO_H
#define ROUTESELECTIONDESCRIPTORINFO_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace OHOS {
namespace NetManagerStandard {

class RouteSelectionDescriptorInfo {
public:
    static const int INVAILID_PDU_SESSION_TYPE;
    static const std::string RSD_SSCMODE;
    static const std::string RSD_SNSSAI;
    static const std::string RSD_DNN;
    static const std::string RSD_PDU_SESSION_TYPE;
    static const uint8_t MATCH_ALL;
    RouteSelectionDescriptorInfo()
        : mSscMode(0), mDnn(""), mSnssai(""), mIsMatchAll(false), mPduSessionType(0)
    {
        mSscMode = 0;
        mDnn = "";
        mSnssai = "";
        mIsMatchAll = false;
        mPduSessionType = 0;
    }

    RouteSelectionDescriptorInfo(uint8_t sscMode, const std::string& dnn,
        const std::string& snssai, bool isMatchAll, int pduSessionType);
    class Builder {
    public:
        Builder()
            : mSscMode(0), mDnn(""), mSnssai(""), mIsMatchAll(false), mPduSessionType(0)
    {}

    Builder& setRSDSscMode(uint8_t sscMode)
    {
        mSscMode = sscMode;
        return *this;
    }

    Builder& setRSDDnn(const std::string& ids)
    {
        mDnn = ids;
        return *this;
    }

    Builder& setRSDSnssai(const std::string& ids)
    {
        mSnssai = ids;
        return *this;
    }

    Builder& setIsMatchAll(bool isMatchAll)
    {
        mIsMatchAll = isMatchAll;
        return *this;
    }

    Builder& setPduSessionType(int PduSessionType)
    {
        mPduSessionType = PduSessionType;
        return *this;
    }

    RouteSelectionDescriptorInfo build()
    {
        RouteSelectionDescriptorInfo rsdinfo;
        rsdinfo.mSscMode = mSscMode;
        rsdinfo.mDnn = mDnn;
        rsdinfo.mSnssai = mSnssai;
        rsdinfo.mIsMatchAll = mIsMatchAll;
        rsdinfo.mPduSessionType = mPduSessionType;
        return rsdinfo;
    }
    private:
        uint8_t mSscMode;
        std::string mDnn;
        std::string mSnssai;
        bool mIsMatchAll;
        int mPduSessionType;
    };

    static RouteSelectionDescriptorInfo makeRouteSelectionDescriptor(std::map<std::string, std::string>& data)
    {
        NETMGR_EXT_LOG_I("makeRouteSelectionDescriptor");
        int sscMode = 0;
        if (data.find(RSD_SSCMODE) != data.end()) {
            sscMode = static_cast<int>(std::stoi(data[RSD_SSCMODE]));
            NETMGR_EXT_LOG_I("makeRouteSelectionDescriptor sscMode = %{public}d", sscMode);
        }
        std::string dnn = "";
        if (data.find(RSD_DNN) != data.end()) {
            dnn = data[RSD_DNN];
            NETMGR_EXT_LOG_I("makeRouteSelectionDescriptor dnn = %{public}s", dnn.c_str());
        }
        std::string snssai = "";
        if (data.find(RSD_SNSSAI) != data.end()) {
            snssai = data[RSD_SNSSAI];
            NETMGR_EXT_LOG_I("makeRouteSelectionDescriptor snssai = %{public}s", snssai.c_str());
        }
        int pduSessionType = INVAILID_PDU_SESSION_TYPE;
        if (data.find(RSD_PDU_SESSION_TYPE) != data.end()) {
            pduSessionType = std::stoi(data[RSD_PDU_SESSION_TYPE]);
            NETMGR_EXT_LOG_I("makeRouteSelectionDescriptor pduSessionType = %{public}d", pduSessionType);
        }
        bool isMatchAll = false;
        if (data.find(TrafficDescriptorsInfo::TDS_ROUTE_BITMAP) != data.end()) {
            isMatchAll = (std::stoi(data[TrafficDescriptorsInfo::TDS_ROUTE_BITMAP]) & MATCH_ALL) != 0;
            NETMGR_EXT_LOG_I("makeRouteSelectionDescriptor isMatchAll = %{public}d", isMatchAll ? 1 : 0);
        }
        RouteSelectionDescriptorInfo::Builder builder;
        builder.setRSDSscMode(sscMode);
        builder.setRSDDnn(dnn);
        builder.setRSDSnssai(snssai);
        builder.setIsMatchAll(isMatchAll);
        builder.setPduSessionType(pduSessionType);
        return builder.build();
    }

    uint8_t getSscMode() const
    {
        return mSscMode;
    }

    std::string getSnssai() const
    {
        return mSnssai;
    }

    std::string getDnn() const
    {
        return mDnn;
    }
    int getPduSessionType() const
    {
        return mPduSessionType;
    }

    bool isMatchAll() const
    {
        return mIsMatchAll;
    }

    bool operator==(const RouteSelectionDescriptorInfo& other) const
    {
        NETMGR_EXT_LOG_I("mSscMode = %{public}d, other.mSscMode = %{public}d", mSscMode, other.mSscMode);
        NETMGR_EXT_LOG_I("mDnn = %{public}s, other.mDnn = %{public}s", mDnn.c_str(), other.mDnn.c_str());
        NETMGR_EXT_LOG_I("mSnssai = %{public}s, other.mSnssai = %{public}s", mSnssai.c_str(), other.mSnssai.c_str());
        NETMGR_EXT_LOG_I("mIsMatchAll = %{public}d, other.mIsMatchAll = %{public}d", mIsMatchAll, other.mIsMatchAll);
        NETMGR_EXT_LOG_I("mPduSessionType = %{public}d, other.mPduSessionType = %{public}d",
            mPduSessionType, other.mPduSessionType);
        return mSscMode == other.mSscMode && mDnn == other.mDnn && mSnssai == other.mSnssai
            && mIsMatchAll == other.mIsMatchAll && mPduSessionType == other.mPduSessionType;
    }

private:
    uint8_t mSscMode;
    std::string mDnn;
    std::string mSnssai;
    bool mIsMatchAll;
    int mPduSessionType;
};

const inline int RouteSelectionDescriptorInfo::INVAILID_PDU_SESSION_TYPE = -1;
const inline std::string RouteSelectionDescriptorInfo::RSD_SSCMODE = "sscMode";
const inline std::string RouteSelectionDescriptorInfo::RSD_SNSSAI = "sNssai";
const inline std::string RouteSelectionDescriptorInfo::RSD_DNN = "dnn";
const inline std::string RouteSelectionDescriptorInfo::RSD_PDU_SESSION_TYPE = "pduSessionType";
const inline uint8_t RouteSelectionDescriptorInfo::MATCH_ALL = 1;
inline RouteSelectionDescriptorInfo::RouteSelectionDescriptorInfo(uint8_t sscMode, const std::string& dnn,
    const std::string& snssai, bool isMatchAll, int pduSessionType) : mSscMode(sscMode), mDnn(dnn), mSnssai(snssai),
    mIsMatchAll(isMatchAll), mPduSessionType(pduSessionType) {}

} // namespace NetManagerStandard
} // namespace OHOS

#endif  // ROUTESELECTIONDESCRIPTORINFO_H
