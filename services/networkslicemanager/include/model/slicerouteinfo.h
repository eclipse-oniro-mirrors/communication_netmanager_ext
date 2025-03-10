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

#ifndef SLICEROUTEINFO_H
#define SLICEROUTEINFO_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace OHOS {
namespace NetManagerStandard {

class SliceRouteInfo {
public:
    static const std::string TAG;
    static const std::string SEPARATOR_FOR_NORMAL_DATA;
    static const uint8_t MATCH_ALL;

    bool isBindCompleted(int uid, const FqdnIps fqdnIps, TrafficDescriptorsInfo::RouteBindType routeBindType) const
    {
        if (routeBindType == TrafficDescriptorsInfo::INVALID_TDS) {
            return true;
        }
        switch (routeBindType) {
            case TrafficDescriptorsInfo::UID_TDS:
                return isUidBindCompleted(uid);
            case TrafficDescriptorsInfo::IP_TDS:
                return isNoNewFqdnIp(fqdnIps);
            case TrafficDescriptorsInfo::UID_IP_TDS:
                return isUidBindCompleted(uid) && isNoNewFqdnIp(fqdnIps);
            default:
                NETMGR_EXT_LOG_I("Invalid TrafficDescriptors RouteBindType.");
        }
        return true;
    }

    void clearUids()
    {
        mUids.clear();
    }

    void addUid(int uid)
    {
        mUids.push_back(uid);
    }

    void removeUid(int uid)
    {
        auto it = std::find(mUids.begin(), mUids.end(), uid);
        if (it != mUids.end()) {
            mUids.erase(it);
        }
    }

    std::string getUidsStr()
    {
        std::stringstream ss;
        for (const int& uid : mUids) {
            ss << uid << SEPARATOR_FOR_NORMAL_DATA;
        }
        return ss.str();
    }

    void addUsedUid(int uid)
    {
        mUsedUids.push_back(uid);
    }

    void clearUsedUids()
    {
        mUsedUids.clear();
    }

    void removeUsedUid(int uid)
    {
        auto it = std::find(mUsedUids.begin(), mUsedUids.end(), uid);
        if (it != mUsedUids.end()) {
            mUsedUids.erase(it);
        }
    }

    void clearWaittingFqdnIps()
    {
        mWaittingFqdnIps.clear();
    }

    std::shared_ptr<FqdnIps> getFqdnIps() const
    {
        return std::make_shared<FqdnIps>(mFqdnIps);
    }

    void setFqdnIps(FqdnIps fqdnIps)
    {
        mFqdnIps = fqdnIps;
    }

    const std::vector<FqdnIps> getWaittingFqdnIps() const
    {
        return mWaittingFqdnIps;
    }

    void setWaittingFqdnIps(const std::vector<FqdnIps>& waittingFqdnIps)
    {
        if (!waittingFqdnIps.empty()) {
            mWaittingFqdnIps.clear();
            for (const auto& fqdnIp : waittingFqdnIps) {
            mWaittingFqdnIps.push_back(fqdnIp);
        }
        }
    }

    std::vector<int> getUsedUids() const
    {
        return mUsedUids;
    }

    std::vector<int> getUids() const
    {
        return mUids;
    }

    void setUids(const std::set<int>& uids)
    {
        if (!uids.empty()) {
            mUids.clear();
            for (const auto& uid : uids) {
                mUids.push_back(uid);
            }
        }
    }

    void addSignedUid(int uid)
    {
        mSignedUids.push_back(uid);
    }

    void removeSignedUid(int uid)
    {
        auto it = std::find(mSignedUids.begin(), mSignedUids.end(), uid);
        if (it != mSignedUids.end()) {
            mSignedUids.erase(it);
        }
    }

    std::vector<int> getSignedUids() const
    {
        return mSignedUids;
    }

    bool isNoNewFqdnIp(const FqdnIps& fqdnIps) const
    {
        return mFqdnIps.isFqdnIpsEmpty() || mFqdnIps.getNewFqdnIps(fqdnIps).isFqdnIpsEmpty();
    }

    bool isUidBindCompleted(int uid) const
    {
        return (std::find(mUids.begin(), mUids.end(), uid) == mUids.end()) ||
               (std::find(mSignedUids.begin(), mSignedUids.end(), uid) == mSignedUids.end());
    }

private:
    std::vector<int> mUids;
    std::vector<int> mUsedUids;
    std::vector<int> mSignedUids;
    std::vector<FqdnIps> mWaittingFqdnIps;
    FqdnIps mFqdnIps;
};

const inline std::string SliceRouteInfo::TAG = "SliceRouteInfo";
const inline std::string SliceRouteInfo::SEPARATOR_FOR_NORMAL_DATA = ",";
const inline uint8_t SliceRouteInfo::MATCH_ALL = 1;

} // namespace NetManagerStandard
} // namespace OHOS

#endif  // SLICEROUTEINFO_H
