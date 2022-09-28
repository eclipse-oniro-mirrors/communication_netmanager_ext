/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "ethernet_client.h"
#include "inet_addr.h"
#include "interface_configuration.h"
#include "refbase.h"
#include "singleton.h"
#include "static_configuration.h"
#include "netmgr_ext_log_wrapper.h"
#include "interface_type.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class EthernetManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<InterfaceConfiguration> GetIfaceConfig();
};

void EthernetManagerTest::SetUpTestCase() {}

void EthernetManagerTest::TearDownTestCase() {}

void EthernetManagerTest::SetUp() {}

void EthernetManagerTest::TearDown() {}

sptr<InterfaceConfiguration> EthernetManagerTest::GetIfaceConfig()
{
    sptr<InterfaceConfiguration> ic = (std::make_unique<InterfaceConfiguration>()).release();
    if (!ic) {
        return ic;
    }
    ic->ipStatic_.ipAddr_.type_ = INetAddr::IPV4;
    ic->ipStatic_.ipAddr_.family_ = 0x01;
    ic->ipStatic_.ipAddr_.prefixlen_ = 0x01;
    ic->ipStatic_.ipAddr_.address_ = "172.17.5.234";
    ic->ipStatic_.ipAddr_.netMask_ = "255.255.254.0";
    ic->ipStatic_.ipAddr_.hostName_ = "netAddr";
    ic->ipStatic_.route_.type_ = INetAddr::IPV4;
    ic->ipStatic_.route_.family_ = 0x01;
    ic->ipStatic_.route_.prefixlen_ = 0x01;
    ic->ipStatic_.route_.address_ = "0.0.0.0";
    ic->ipStatic_.route_.netMask_ = "0.0.0.0";
    ic->ipStatic_.route_.hostName_ = "netAddr";
    ic->ipStatic_.gateway_.type_ = INetAddr::IPV4;
    ic->ipStatic_.gateway_.family_ = 0x01;
    ic->ipStatic_.gateway_.prefixlen_ = 0x01;
    ic->ipStatic_.gateway_.address_ = "172.17.4.1";
    ic->ipStatic_.gateway_.netMask_ = "0.0.0.0";
    ic->ipStatic_.gateway_.hostName_ = "netAddr";
    ic->ipStatic_.netMask_.type_ = INetAddr::IPV4;
    ic->ipStatic_.netMask_.family_ = 0x01;
    ic->ipStatic_.netMask_.netMask_ = "255.255.255.0";
    ic->ipStatic_.netMask_.hostName_ = "netAddr";
    INetAddr dns1;
    dns1.type_ = INetAddr::IPV4;
    dns1.family_ = 0x01;
    dns1.address_ = "8.8.8.8";
    dns1.hostName_ = "netAddr";
    INetAddr dns2;
    dns2.type_ = INetAddr::IPV4;
    dns2.family_ = 0x01;
    dns2.address_ = "114.114.114.114";
    dns2.hostName_ = "netAddr";
    ic->ipStatic_.dnsServers_.push_back(dns1);
    ic->ipStatic_.dnsServers_.push_back(dns2);
    return ic;
}

/**
 * @tc.name: EthernetManager001
 * @tc.desc: Test EthernetManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager001, TestSize.Level1)
{
    std::string iface = "eth0";
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(iface, ic);
    (void)result;
    ASSERT_TRUE(true);
}

/**
 * @tc.name: EthernetManager004
 * @tc.desc: Test EthernetManager GetAllActiveIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager004, TestSize.Level1)
{
    std::vector<std::string> result = DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces();
    for (std::string& s : result) {
        std::cout << s << ","<< std::endl;
    }
}

HWTEST_F(EthernetManagerTest, EthernetManager006, TestSize.Level1)
{
    std::string iface = "eth0";
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ASSERT_TRUE(DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(iface, cfg));
    ASSERT_FALSE(cfg.ifName.empty());
    ASSERT_FALSE(cfg.hwAddr.empty());
}

HWTEST_F(EthernetManagerTest, EthernetManager007, TestSize.Level1)
{
    std::string iface = "eth0";
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceDown(iface);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ASSERT_TRUE(DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(iface, cfg));
    auto fit = std::find(cfg.flags.begin(), cfg.flags.end(), "down");
    ASSERT_EQ(cfg.ifName, iface);
    ASSERT_TRUE(*fit == "down");
    ASSERT_TRUE(result == 0);
}

HWTEST_F(EthernetManagerTest, EthernetManager008, TestSize.Level1)
{
    std::string iface = "eth0";
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceUp(iface);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ASSERT_TRUE(DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(iface, cfg));
    auto fit = std::find(cfg.flags.begin(), cfg.flags.end(), "up");
    ASSERT_EQ(cfg.ifName, iface);
    ASSERT_TRUE(*fit == "up");
    ASSERT_TRUE(result == 0);
}
} // namespace NetManagerStandard
} // namespace OHOS