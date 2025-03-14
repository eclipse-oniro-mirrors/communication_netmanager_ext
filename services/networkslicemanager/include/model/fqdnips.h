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

#ifndef FQDNIPS_H
#define FQDNIPS_H

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include "inet_addr.h"
#include "networksliceutil.h"

namespace OHOS {
namespace NetManagerStandard {

class FqdnIps {
public:
    static const std::string TAG;
    static const std::string CHAR_ENCODING;

    static const std::string IPV4_MASK;
    static const std::string IPV6_PREFIX;

    void mergeFqdnIps(const FqdnIps& newFqdnIps)
    {
        if (newFqdnIps.mIpv4Addr.size() > 0) {
            for (INetAddr ip : newFqdnIps.mIpv4Addr) {
                mIpv4Addr.insert(ip);
            }
        }
        if (newFqdnIps.mIpv6Addr.size() > 0) {
            for (INetAddr ip : newFqdnIps.mIpv6Addr) {
                mIpv6Addr.insert(ip);
            }
        }
    }

    bool hasNewFqdnIps(const FqdnIps& fqdnIps) const
    {
        bool containsAllIpv4 = true;
        bool containsAllIpv6 = true;

        for (const auto& ipv4 : fqdnIps.mIpv4Addr) {
            bool found = false;
            for (const auto& myIpv4 : mIpv4Addr) {
                if (ipv4 == myIpv4) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                containsAllIpv4 = false;
                break;
            }
        }

        for (const auto& ipv6 : fqdnIps.mIpv6Addr) {
            bool found = false;
            for (const auto& myIpv6 : mIpv6Addr) {
                if (ipv6 == myIpv6) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                containsAllIpv6 = false;
                break;
            }
        }
        return !containsAllIpv4 || !containsAllIpv6;
    }

    FqdnIps getNewFqdnIps(const FqdnIps& fqdnIps) const
    {
        if (fqdnIps.isFqdnIpsEmpty()) {
            return FqdnIps();
        }
        FqdnIps newFqdnIps;
        for (const INetAddr& ip : fqdnIps.mIpv4Addr) {
            if (std::find(mIpv4Addr.begin(), mIpv4Addr.end(), ip) == mIpv4Addr.end()) {
                newFqdnIps.mIpv4Addr.insert(ip);
            }
        }
        for (const INetAddr& ip : fqdnIps.mIpv6Addr) {
            if (std::find(mIpv6Addr.begin(), mIpv6Addr.end(), ip) == mIpv6Addr.end()) {
                newFqdnIps.mIpv6Addr.insert(ip);
            }
        }
        return newFqdnIps;
    }

    bool isFqdnIpsEmpty() const
    {
        return mIpv4Addr.empty() && mIpv6Addr.empty();
    }

    std::vector<uint8_t> setIpv4AddrAndMask()
    {
        std::string ipv4AddrAndMask;
        for (const INetAddr& ip : mIpv4Addr) {
            ipv4AddrAndMask += ip.address_;
            ipv4AddrAndMask += IPV4_MASK;
        }
        mIpv4AddrAndMask = ConvertstringTouInt8Vector(ipv4AddrAndMask);
        return mIpv4AddrAndMask;
    }

    std::vector<uint8_t> setIpv6AddrAndPrefix()
    {
        std::string ipv6AddrAndPrefix;
        for (const INetAddr& ip : mIpv6Addr) {
            ipv6AddrAndPrefix += ip.address_;
            ipv6AddrAndPrefix += IPV6_PREFIX;
        }
        mIpv6AddrAndPrefix = ConvertstringTouInt8Vector(ipv6AddrAndPrefix);
        return mIpv6AddrAndPrefix;
    }

    std::set<INetAddr> getIpv4Addr() const
    {
        return mIpv4Addr;
    }

    std::set<INetAddr> getIpv6Addr() const
    {
        return mIpv6Addr;
    }

    size_t getIpv4Num() const
    {
        return mIpv4Addr.size();
    }

    size_t getIpv6Num() const
    {
        return mIpv6Addr.size();
    }

    std::vector<uint8_t> getIpv4AddrAndMask() const
    {
        return mIpv4AddrAndMask;
    }

    std::vector<uint8_t> getIpv6AddrAndPrefix() const
    {
        return mIpv6AddrAndPrefix;
    }

    void setIpv4Addr(std::set<INetAddr> ipv4Addr)
    {
        mIpv4Addr = ipv4Addr;
    }

    void setIpv6Addr(std::set<INetAddr> ipv6Addr)
    {
        mIpv6Addr = ipv6Addr;
    }

    bool operator==(const FqdnIps& other) const
    {
        return mIpv4Addr == other.mIpv4Addr && mIpv6Addr == other.mIpv6Addr;
    }
     
    bool operator<(const FqdnIps& other) const
    {
        if (mIpv4Addr.size() < other.mIpv4Addr.size()) return true;
        if (mIpv4Addr.size() > other.mIpv4Addr.size()) return false;
        auto it1 = mIpv4Addr.begin();
        auto it2 = other.mIpv4Addr.begin();
        while (it1 != mIpv4Addr.end() && it2 != other.mIpv4Addr.end()) {
            if (*it1 < *it2) return true;
            if (!(*it1 == *it2)) return false;
            ++it1;
            ++it2;
        }

        if (mIpv6Addr.size() < other.mIpv6Addr.size()) return true;
        if (mIpv6Addr.size() > other.mIpv6Addr.size()) return false;
        it1 = mIpv6Addr.begin();
        it2 = other.mIpv6Addr.begin();
        while (it1 != mIpv6Addr.end() && it2 != other.mIpv6Addr.end()) {
            if (*it1 < *it2) return true;
            if (!(*it1 == *it2)) return false;
            ++it1;
            ++it2;
        }
        return false;
    }

private:
    std::set<INetAddr> mIpv4Addr;
    std::set<INetAddr> mIpv6Addr;

    std::vector<uint8_t> mIpv4AddrAndMask;
    std::vector<uint8_t> mIpv6AddrAndPrefix;
};

const inline std::string FqdnIps::TAG = "FqdnIps";
const inline std::string FqdnIps::CHAR_ENCODING = "ISO_8859_1";
const inline std::string FqdnIps::IPV4_MASK = "0.0.0.0";
const inline std::string FqdnIps::IPV6_PREFIX = "::1";

} // namespace NetManagerStandard
} // namespace OHOS

#endif  // FQDNIPS_H
