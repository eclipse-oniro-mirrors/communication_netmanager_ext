/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define private public
#include "dev_interface_state.h"
#include "ethernet_lan_management.h"

namespace OHOS {
namespace NetManagerStandard {

class EthernetLanManagementTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void EthernetLanManagementTest::SetUpTestCase() {}

void EthernetLanManagementTest::TearDownTestCase() {}

void EthernetLanManagementTest::SetUp() {}

void EthernetLanManagementTest::TearDown() {}

HWTEST_F(EthernetLanManagementTest, EthernetLanManagement001, TestSize.Level1)
{
    EthernetLanManagement ethernetLanManager;
    sptr<DevInterfaceState> devState = new (std::nothrow) DevInterfaceState();
    devState->SetDevName("eth2");
    
    sptr<InterfaceConfiguration> ic = (std::make_unique<InterfaceConfiguration>()).release();
    ic->mode_ = LAN_STATIC;
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = 0x01;
    ipv4Addr.prefixlen_ = 0x01;
    ipv4Addr.address_ = "192.168.42.1";
    ipv4Addr.netMask_ = "255.255.255.0";
    ipv4Addr.hostName_ = "netAddr";
    ic->ipStatic_.ipAddrList_.push_back(ipv4Addr);
    INetAddr route;
    route.type_ = INetAddr::IPV4;
    route.family_ = 0x01;
    route.prefixlen_ = 0x01;
    route.address_ = "0.0.0.0";
    route.netMask_ = "0.0.0.0";
    route.hostName_ = "netAddr";
    ic->ipStatic_.routeList_.push_back(route);

    devState->SetLancfg(ic);
    int32_t ret = ethernetLanManager.DelIp(*(devState->GetLinkInfo()))
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
