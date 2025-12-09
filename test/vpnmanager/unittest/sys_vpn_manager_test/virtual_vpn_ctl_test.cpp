/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <memory>

#include "route.h"
#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "virtual_vpn_ctl.h"

#include "networkvpn_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class VirtualVpnConnStateCbTest : public IVpnConnStateCb {
public:
    VirtualVpnConnStateCbTest() = default;
    virtual ~VirtualVpnConnStateCbTest() = default;
    void OnVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnIfName,
                               const std::string &vpnIfAddr,
                               const std::string &vpnId, bool isGlobalVpn) override;
    void SendConnStateChanged(const VpnConnectState &state) override;
    void OnMultiVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnId) override;
};

void VirtualVpnConnStateCbTest::OnVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnIfName,
                                                      const std::string &vpnIfAddr,
                                                      const std::string &vpnId, bool isGlobalVpn) {}
void VirtualVpnConnStateCbTest::SendConnStateChanged(const VpnConnectState &state) {}
void VirtualVpnConnStateCbTest::OnMultiVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnId) {}

class VirtualVpnCtlTest : public testing::Test {
public:
    static inline std::unique_ptr<VirtualVpnCtl> control_ = nullptr;
    static void SetUpTestSuite();
};

void VirtualVpnCtlTest::SetUpTestSuite()
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    if (config == nullptr) {
        return;
    }
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    control_ = std::make_unique<VirtualVpnCtl>(config, "pkg", userId, activeUserIds);
    if (control_ == nullptr) {
        return;
    }
}

HWTEST_F(VirtualVpnCtlTest, SetUp001, TestSize.Level1)
{
    control_->vpnConfig_ = nullptr;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(VirtualVpnCtlTest, SetUp002, TestSize.Level1)
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    control_->vpnConfig_ = config;
    EXPECT_EQ(control_->SetUp(true), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(VirtualVpnCtlTest, Destory001, TestSize.Level1)
{
    EXPECT_EQ(control_->Destory(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VirtualVpnCtlTest, NotifyConnectState001, TestSize.Level1)
{
    control_->connChangedCb_ = nullptr;
    control_->NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    EXPECT_EQ(control_->netSupplierId_, 0);
}

HWTEST_F(VirtualVpnCtlTest, NotifyConnectState002, TestSize.Level1)
{
    std::shared_ptr<IVpnConnStateCb> cb = std::make_shared<VirtualVpnConnStateCbTest>();
    control_->connChangedCb_ = cb;
#ifdef SUPPORT_SYSVPN
    control_->multiVpnInfo_ = nullptr;
#endif
    control_->NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    EXPECT_EQ(control_->netSupplierId_, 0);
}

HWTEST_F(VirtualVpnCtlTest, NotifyConnectState003, TestSize.Level1)
{
    std::shared_ptr<IVpnConnStateCb> cb = std::make_shared<VirtualVpnConnStateCbTest>();
    control_->connChangedCb_ = cb;
#ifdef SUPPORT_SYSVPN
    sptr<MultiVpnInfo> vpnInfo = new (std::nothrow) MultiVpnInfo();
    vpnInfo->vpnId = "test";
    control_->multiVpnInfo_ = vpnInfo;
#endif
    control_->NotifyConnectState(VpnConnectState::VPN_CONNECTED);
    EXPECT_EQ(control_->netSupplierId_, 0);
}

} // namespace NetManagerStandard
} // namespace OHOS