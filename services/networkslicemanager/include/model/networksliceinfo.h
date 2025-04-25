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

#ifndef NETWORKSLCIEINFO_H
#define NETWORKSLCIEINFO_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include "trafficdescriptorsinfo.h"
#include "routeselectiondescriptorInfo.h"
#include "slicerouteinfo.h"
#include "net_specifier.h"
#include "net_conn_callback.h"

namespace OHOS {
namespace NetManagerStandard {

class NetworkSliceInfo {
public:
    enum class ParaType {
        NETWORK_CALLBACK,
        NETWORK_REQUEST,
        ROUTE_SELECTION_DESCRIPTOR,
        NETWORK_REQUEST_ID
    };
    static const int INVALID_NET_ID;
    static const std::string TAG;

    bool isBindCompleted(int uid, const FqdnIps& fqdnIps, const std::shared_ptr<TrafficDescriptorsInfo>& tds) const
    {
        // Empty mTrafficDescriptors is an exception scenario, can't bind any route for this networkSlice
        if (tds == nullptr) {
            return true;
        }
        auto it = mSliceRouteInfos.find(*tds);
        if (it == mSliceRouteInfos.end()) {
            return false;
        } else {
            SliceRouteInfo sri = it->second;
            return sri.isBindCompleted(uid, fqdnIps, tds->getRouteBindType());
        }
    }

    void clear()
    {
        mNetId = INVALID_NET_ID;
        mNetworkCallback = nullptr;
        mTempTrafficDescriptors = nullptr;
        mRouteSelectionDescriptor = nullptr;
        mNetworkRequests.clear();
        mSliceRouteInfos.clear();
    }

    void clearUsedUids()
    {
        for (auto& sri : mSliceRouteInfos) {
            sri.second.clearUsedUids();
        }
    }

    void clearUids()
    {
        for (auto& sri : mSliceRouteInfos) {
            sri.second.clearUids();
        }
    }

    bool isRightNetworkSliceRsd(RouteSelectionDescriptorInfo rsd, ParaType type)
    {
        NETMGR_EXT_LOG_I("isRightNetworkSliceRsd");
        std::shared_ptr<RouteSelectionDescriptorInfo> rsdinfo = getRouteSelectionDescriptor();
        if (rsdinfo != nullptr) {
            NETMGR_EXT_LOG_I("isRightNetworkSliceRsd, rsdinfo != nullptr");
            return rsd == *rsdinfo;
        }
        return false;
    }

    bool isRightNetworkSliceNull(ParaType type)
    {
        return nullptr == getRouteSelectionDescriptor();
    }

    bool isRightNetworkSliceNetCap(NetCap netCap)
    {
        return netCap == mNetworkCapability;
    }

    sptr<NetSpecifier> getNetworkRequestByRequestId(int requestId)
    {
        return mNetworkRequests[requestId];
    }

    void mergeFqdnIps(const FqdnIps& newFqdnIps, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr && sri->getFqdnIps() != nullptr) {
            sri->getFqdnIps()->mergeFqdnIps(newFqdnIps);
        }
    }

    void clearWaittingFqdnIps(const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->clearWaittingFqdnIps();
        }
    }

    bool isMatchAll() const
    {
        if (mRouteSelectionDescriptor == nullptr) {
            return false;
        }
        return mRouteSelectionDescriptor->isMatchAll();
    }

    bool isIpTriad(std::shared_ptr<TrafficDescriptorsInfo>& tds) const
    {
        if (tds == nullptr) {
            return false;
        }
        return tds->isIpTriad();
    }

    int getNetId() const
    {
        return mNetId;
    }

    void setNetId(int netId)
    {
        mNetId = netId;
    }

    int getNetworkCapability() const
    {
        return mNetworkCapability;
    }

    void setNetworkCapability(int networkCapability)
    {
        mNetworkCapability = networkCapability;
    }

    sptr<NetConnCallback> getNetworkCallback() const
    {
        return mNetworkCallback;
    }
 
    void setNetworkCallback(sptr<NetConnCallback> callback)
    {
        mNetworkCallback = callback;
    }

    sptr<NetSpecifier> getNetworkRequest() const
    {
        return mNetworkRequest;
    }

    void setNetworkRequest(sptr<NetSpecifier> networkRequest)
    {
        mNetworkRequest = networkRequest;
    }

    std::shared_ptr<RouteSelectionDescriptorInfo> getRouteSelectionDescriptor() const
    {
        return mRouteSelectionDescriptor;
    }

    void setRouteSelectionDescriptor(RouteSelectionDescriptorInfo routeSelectionDescriptor)
    {
        mRouteSelectionDescriptor = std::make_shared<RouteSelectionDescriptorInfo>(routeSelectionDescriptor);
    }

    std::vector<int> getUsedUids(const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            return sri->getUsedUids();
        }
        return std::vector<int>();
    }

    void addUsedUid(int uid, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->addUsedUid(uid);
        }
    }

    void removeUsedUid(int uid, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->removeUsedUid(uid);
        }
    }

    bool isInUsedUids(int uid, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            std::vector<int> uids = sri->getUsedUids();
            return (std::find(uids.begin(), uids.end(), uid) != uids.end());
        }
        return false;
    }

    bool isUsedUidEmpty(const TrafficDescriptorsInfo& tds) const
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            return sri->getUsedUids().empty();
        }
        return true;
    }

    void addUsedUids(const std::set<int>& uids, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr && !uids.empty()) {
            for (const auto& uid : uids) {
                sri->addUsedUid(uid);
            }
        }
    }

    std::vector<int> getUids(const TrafficDescriptorsInfo& tds) const
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            return sri->getUids();
        }
        return std::vector<int>();
    }

    void addUid(int uid, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->addUid(uid);
        }
    }

    void addUids(const std::set<int>& uids, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr && !uids.empty()) {
            for (const auto& uid : uids) {
                sri->addUid(uid);
            }
        }
    }

    void removeUid(int uid, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->removeUid(uid);
        }
    }

    void replaceUids(const TrafficDescriptorsInfo& tds, const std::set<int>& uids)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr && !sri->getUids().empty()) {
            sri->setUids(uids);
        }
    }

    std::shared_ptr<FqdnIps> getFqdnIps(const TrafficDescriptorsInfo& tds) const
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            return sri->getFqdnIps();
        }
        return {};
    }

    void setFqdnIps(const FqdnIps& fqdnIps, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->setFqdnIps(fqdnIps);
        }
    }

    std::vector<FqdnIps> getWaittingFqdnIps(const TrafficDescriptorsInfo& tds) const
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            return sri->getWaittingFqdnIps();
        }
        return std::vector<FqdnIps>();
    }

    void setWaittingFqdnIps(const std::vector<FqdnIps>& waittingFqdnIps, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->setWaittingFqdnIps(waittingFqdnIps);
        }
    }

    void cacheTrafficDescriptors(const TrafficDescriptorsInfo& tds)
    {
        NETMGR_EXT_LOG_E("cacheTrafficDescriptors UrspPrecedence = %{public}d, AppIds = %{public}s",
            tds.getUrspPrecedence(), tds.getAppIds().c_str());
        
        SliceRouteInfo sliceRouteInfo;
        mSliceRouteInfos[tds] = sliceRouteInfo;
    }

    void setTempTrafficDescriptors(const TrafficDescriptorsInfo& tds)
    {
        mTempTrafficDescriptors = std::make_shared<TrafficDescriptorsInfo>(tds);
    }

    std::shared_ptr<TrafficDescriptorsInfo> getTempTrafficDescriptors() const
    {
        return mTempTrafficDescriptors;
    }

    std::shared_ptr<SliceRouteInfo> getSliceRouteInfo(const TrafficDescriptorsInfo& tds) const
    {
        if (!tds.isUidRouteBindType()) {
            return nullptr;
        }
        auto it = mSliceRouteInfos.find(tds);
        if (it != mSliceRouteInfos.end()) {
            return std::make_shared<SliceRouteInfo>(it->second);
        } else {
            return nullptr;
        }
    }
    
    std::vector<TrafficDescriptorsInfo> getTrafficDescriptorsInfos() const
    {
        std::vector<TrafficDescriptorsInfo> trafficDescriptorsInfos;
        for (auto iter = mSliceRouteInfos.begin(); iter != mSliceRouteInfos.end(); iter++) {
            trafficDescriptorsInfos.push_back(iter->first);
        }
        return trafficDescriptorsInfos;
    }

    std::map<TrafficDescriptorsInfo, SliceRouteInfo> getSliceRouteInfos() const
    {
        return mSliceRouteInfos;
    }

    bool isUidRouteBindType(const std::shared_ptr<TrafficDescriptorsInfo>& tds) const
    {
        if (tds == nullptr) {
            return false;
        }
        return tds->isUidRouteBindType();
    }

    void clearSliceRouteInfos()
    {
        mSliceRouteInfos.clear();
    }

    std::vector<int> getSignedUids(const TrafficDescriptorsInfo& tds) const
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            return sri->getSignedUids();
        }
        return std::vector<int>();
    }

    void addSignedUid(int uid, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->addSignedUid(uid);
        }
    }

    void removeSignedUid(int uid, const TrafficDescriptorsInfo& tds)
    {
        std::shared_ptr<SliceRouteInfo> sri = getSliceRouteInfo(tds);
        if (sri != nullptr) {
            sri->removeSignedUid(uid);
        }
    }
private:
    int mNetId = INVALID_NET_ID;
    int mNetworkCapability;
    sptr<NetConnCallback> mNetworkCallback;
    sptr<NetSpecifier> mNetworkRequest;
    std::shared_ptr<TrafficDescriptorsInfo> mTempTrafficDescriptors;
    std::shared_ptr<RouteSelectionDescriptorInfo> mRouteSelectionDescriptor;
    std::map<int, sptr<NetSpecifier>> mNetworkRequests;
    std::map<TrafficDescriptorsInfo, SliceRouteInfo> mSliceRouteInfos;
};

const inline int NetworkSliceInfo::INVALID_NET_ID = -1;
const inline std::string NetworkSliceInfo::TAG = "NetworkSliceInfo";

} // namespace NetManagerStandard
} // namespace OHOS

#endif  // NETWORKSLCIEINFO_H
