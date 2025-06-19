/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_MULTI_VPN_HELPER_H
#define OHOS_MULTI_VPN_HELPER_H
#include <string>
#include <vector>

#include "sysvpn_config.h"
#include "refbase.h"
#include "net_manager_ext_constants.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetsysNative;
struct MultiVpnInfo : RefBase {
    std::string vpnId;
    std::string ifName;
    std::string bundleName;
    int32_t ifNameId;
    int32_t callingUid;
    int32_t userId;
    VpnConnectState vpnConnectState = VpnConnectState::VPN_DISCONNECTED;
    bool isVpnExtCall = true;
};

class MultiVpnHelper {
public:
    static MultiVpnHelper &GetInstance();
    int32_t GetNewIfNameId();
    int32_t CreateMultiVpnInfo(const std::string &vpnId, int32_t vpnType, sptr<MultiVpnInfo> &info);
    int32_t AddMultiVpnInfo(const sptr<MultiVpnInfo> &info);
    int32_t DelMultiVpnInfo(const sptr<MultiVpnInfo> &info);
    bool StartIpsec();
    void StopIpsec();
    bool StartL2tp();
    void StopL2tp();
    bool IsConnectedStage(const std::string &stage);
private:
    MultiVpnHelper();
    bool IsOpenvpnConnectedStage(const std::string &stage);
    std::vector<sptr<MultiVpnInfo>> multiVpnInfos_;
    int32_t ipsecStartedCount_ = 0;
    int32_t l2tpStartedCount_ = 0;
};
} // namespace MultiVpnHelper
} // namespace OHOS
#endif // OHOS_MULTI_VPN_HELPER_H