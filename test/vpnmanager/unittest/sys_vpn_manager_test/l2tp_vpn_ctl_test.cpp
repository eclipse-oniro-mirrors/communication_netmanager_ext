/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifdef GTEST_API_
#define private public
#endif
#include "vpn_config.h"
#include "l2tp_vpn_ctl.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;

class L2tpVpnCtlTest : public testing::Test {
public:
    static inline std::unique_ptr<IpsecVpnCtl> l2tpControl_ = nullptr;
    static void SetUpTestSuite();
};

void L2tpVpnCtlTest::SetUpTestSuite()
{
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    if (config == nullptr) {
        return;
    }
    int32_t userId = 0;
    std::vector<int32_t> activeUserIds;
    l2tpControl_ = std::make_unique<L2tpVpnCtl>(config, "pkg", userId, activeUserIds);
}
HWTEST_F(L2tpVpnCtlTest, SetUp001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(l2tpControl_->SetUp(), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(L2tpVpnCtlTest, Destroy001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(l2tpControl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, IsInternalVpn001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    EXPECT_EQ(l2tpControl_->IsInternalVpn(), true);
}

HWTEST_F(L2tpVpnCtlTest, GetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    sptr<SysVpnConfig> resConfig = nullptr;
    EXPECT_EQ(l2tpControl_->GetConnectedSysVpnConfig(resConfig), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(L2tpVpnCtlTest, NotifyConnectStageTest001, TestSize.Level1)
{
    if (l2tpControl_ == nullptr) {
        return;
    }
    std::string stage = "connect";
    int32_t errorCode = 100;
    EXPECT_EQ(l2tpControl_->NotifyConnectStage(stage, errorCode), NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS