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

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "inet_addr.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "vpn_template_processor.h"
#include "net_manager_constants.h"
 
namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class VpnTemplateProcessorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void VpnTemplateProcessorTest::SetUpTestCase() {}

void VpnTemplateProcessorTest::TearDownTestCase() {}

void VpnTemplateProcessorTest::SetUp() {}

void VpnTemplateProcessorTest::TearDown() {}

 
HWTEST_F(VpnTemplateProcessorTest, BuildConfig001, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = nullptr;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(config), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig002, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = nullptr;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(config), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig003, TestSize.Level1)
{
    sptr<IpsecVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    ASSERT_NE(config, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = "1234";
    config->vpnName_ = "test001";
    config->vpnType_ = 1;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(config), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnTemplateProcessorTest, BuildConfig004, TestSize.Level1)
{
    sptr<L2tpVpnConfig> config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    sptr<INetAddr> netAddr = new (std::nothrow) INetAddr();
    ASSERT_NE(netAddr, nullptr);
    std::string ip = "1.1.1.1";
    netAddr->address_ = ip;
    netAddr->prefixlen_ = 1;
    config->addresses_.push_back(*netAddr);
    config->vpnId_ = "1234";
    config->vpnName_ = "test001";
    config->vpnType_ = 4;
    VpnTemplateProcessor processor;
    EXPECT_EQ(processor.BuildConfig(config), NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS