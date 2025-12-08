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

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class VirtualVpnCtlTest : public testing::Test {
public:
    static void SetUpTestSuite();
};

void VirtualVpnCtlTest::SetUpTestSuite()
{}

HWTEST_F(VirtualVpnCtlTest, TestCreateVirtualVpn, TestSize.Level1)
{
    NetworkVpnClient &vpnClient = NetworkVpnClient::GetInstance();
    auto config = sptr<SysVpnConfig>::MakeSptr();

    config->vpnType_ = VpnType::VIRTUAL_VPN;
    config->vpnId_ = "dcpc_vpn_share_mgr";
    int32_t rc = vpnClient.SetUpVpn(config, true);
    EXPECT_EQ(rc, 0);

    rc = vpnClient.NotifyConnectStage("connect", 0);
    EXPECT_EQ(rc, 0);

    vpnClient.DestoryVpn(dcpcShareVpnName);
}

} // namespace NetManagerStandard
} // namespace OHOS