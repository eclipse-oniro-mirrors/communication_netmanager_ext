/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <memory>

#include <gtest/gtest.h>

#include "net_manager_constants.h"
#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "extended_vpn_ctl.h"

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;

class ExtendedVpnCtlTest : public testing::Test {
public:
    static inline std::unique_ptr<ExtendedVpnCtl> vpnControl_ = nullptr;
    static void SetUpTestSuite();
};

void ExtendedVpnCtlTest::SetUpTestSuite()
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    if (config == nullptr) {
        return;
    }
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    vpnControl_ = std::make_unique<ExtendedVpnCtl>(config, "pkg", userId, activeUserIds);
}

HWTEST_F(ExtendedVpnCtlTest, SetUp001, TestSize.Level1)
{
    EXPECT_EQ(vpnControl_->SetUp(), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(ExtendedVpnCtlTest, SetUp002, TestSize.Level1)
{
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    if (vpnInfo == nullptr) {
        return;
    }
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    if (netAddr == nullptr) {
        return;
    }
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    vpnControl_->vpnConfig_->addresses_.push_back(*netAddr);
    vpnInfo->isVpnExtCall = true;
    vpnInfo->ifNameId = 1;
    vpnInfo->ifName = "test";
    vpnControl_->multiVpnInfo_ = vpnInfo;
    vpnControl_->vpnConfig_->vpnId_ = "1";
    EXPECT_EQ(vpnControl_->SetUp(), NETMANAGER_EXT_ERR_INTERNAL);
    vpnControl_->multiVpnInfo_->localAddress = "1.1.1.1";
    MultiVpnHelper::GetInstance().AddMultiVpnInfo(vpnControl_->multiVpnInfo_);
    EXPECT_EQ(vpnControl_->SetUp(), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(ExtendedVpnCtlTest, Destroy001, TestSize.Level1)
{
    EXPECT_EQ(vpnControl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(ExtendedVpnCtlTest, Destroy002, TestSize.Level1)
{
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    if (vpnInfo == nullptr) {
        return;
    }
    vpnInfo->isVpnExtCall = true;
    vpnInfo->ifNameId = 1;
    vpnControl_->multiVpnInfo_ = vpnInfo;
    vpnControl_->vpnConfig_->vpnId_ = "1";

    EXPECT_EQ(vpnControl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(ExtendedVpnCtlTest, IsInternalVpn, TestSize.Level1)
{
    EXPECT_EQ(vpnControl_->IsInternalVpn(), false);
}

} // namespace NetManagerStandard
} // namespace OHOS
