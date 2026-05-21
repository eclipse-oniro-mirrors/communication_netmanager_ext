/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <cstring>
#include <parcel.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "ethernet_client.h"
#include "ethernet_service.h"
#include "ethernet_management.h"
#include "dev_interface_state.h"
#include "mac_address_info.h"
#include "net_ethernet.h"
#include "net_link_info.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "netmanager_ext_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
constexpr const char *DEV_NAME = "eth0";
constexpr const char *USB_DEV_NAME = "usb0";
constexpr const char *INVALID_IFACE = "wlan0";
} // namespace

class EthernetMacIpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void EthernetMacIpTest::SetUpTestCase() {}

void EthernetMacIpTest::TearDownTestCase() {}

void EthernetMacIpTest::SetUp() {}

void EthernetMacIpTest::TearDown() {}

HWTEST_F(EthernetMacIpTest, OH_Ethernet_GetMacAddress_NullPtr, TestSize.Level1)
{
    int32_t ret = OH_Ethernet_GetMacAddress(nullptr);
    EXPECT_EQ(ret, 2200001);

    int32_t ret2 = OH_Ethernet_GetNetAddress(nullptr);
    EXPECT_EQ(ret2, 2200001);
}

HWTEST_F(EthernetMacIpTest, OH_Ethernet_GetMacAddress_Normal, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    Ethernet_MacAddrInfoList macAddrList;
    memset_s(&macAddrList, sizeof(Ethernet_MacAddrInfoList), 0, sizeof(Ethernet_MacAddrInfoList));
    int32_t ret = OH_Ethernet_GetMacAddress(&macAddrList);
    EXPECT_TRUE(ret == 0 || ret == 2200002 || ret == 2201005 || ret == 201);
}

HWTEST_F(EthernetMacIpTest, OH_Ethernet_GetNetAddress_Normal, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    Ethernet_NetAddrList netAddrList;
    memset_s(&netAddrList, sizeof(Ethernet_NetAddrList), 0, sizeof(Ethernet_NetAddrList));
    int32_t ret = OH_Ethernet_GetNetAddress(&netAddrList);
    EXPECT_TRUE(ret == 0 || ret == 2200002 || ret == 2201005 || ret == 201);
}

HWTEST_F(EthernetMacIpTest, OH_Ethernet_GetMacAddress_NoPermission, TestSize.Level1)
{
    NoPermissionAccessToken token;
    Ethernet_MacAddrInfoList macAddrList;
    memset_s(&macAddrList, sizeof(Ethernet_MacAddrInfoList), 0, sizeof(Ethernet_MacAddrInfoList));
    int32_t ret = OH_Ethernet_GetMacAddress(&macAddrList);
    EXPECT_EQ(ret, 201);
}

HWTEST_F(EthernetMacIpTest, OH_Ethernet_GetNetAddress_NoPermission, TestSize.Level1)
{
    NoPermissionAccessToken token;
    Ethernet_NetAddrList netAddrList;
    memset_s(&netAddrList, sizeof(Ethernet_NetAddrList), 0, sizeof(Ethernet_NetAddrList));
    int32_t ret = OH_Ethernet_GetNetAddress(&netAddrList);
    EXPECT_EQ(ret, 201);
}

} // namespace NetManagerStandard
} // namespace OHOS
