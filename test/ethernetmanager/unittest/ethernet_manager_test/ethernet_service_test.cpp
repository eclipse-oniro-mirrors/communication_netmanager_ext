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

#include "accesstoken_kit.h"
#include "ethernet_client.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "interface_configuration.h"
#include "interface_type.h"
#include "nativetoken_kit.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "static_configuration.h"
#include "token_setproc.h"
#define private public
#define protected public
#include "ethernet_client.h"
#include "ethernet_dhcp_controller.h"
#include "ethernet_service.h"
#include "ethernet_management.h"
#include "ethernet_service_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
namespace {
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
constexpr const char *DEV_NAME = "eth0";

HapInfoParams testInfoParms = {.userID = 1,
                               .bundleName = "ethernet_manager_test",
                               .instIndex = 0,
                               .appIDDesc = "test",
                               .isSystemApp = true};
PermissionDef testPermDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "ethernet_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test network share manager",
    .descriptionId = 1,
};
PermissionStateFull testState = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};
HapPolicyParams testPolicyPrams1 = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef},
    .permStateList = {testState},
};

PermissionDef testPermDef2 = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .bundleName = "ethernet_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test network share manager",
    .descriptionId = 1,
};
PermissionStateFull testState2 = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};
HapPolicyParams testPolicyPrams2 = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef2},
    .permStateList = {testState2},
};
} // namespace
} // namespace

class EtherNetServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    sptr<InterfaceConfiguration> GetIfaceConfig();
    void SetUp();
    void TearDown();
};

sptr<InterfaceConfiguration> EtherNetServiceTest::GetIfaceConfig()
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

void EtherNetServiceTest::SetUpTestCase() {}

void EtherNetServiceTest::TearDownTestCase() {}

void EtherNetServiceTest::SetUp() {}

void EtherNetServiceTest::TearDown() {}

HWTEST_F(EtherNetServiceTest, OnInterfaceAddressUpdatedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string addr;
    std::string ifName;
    int flags = 0;
    int scope = 0;
    int ret = globalinterfacestatecallback.OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceAddressRemovedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string addr;
    std::string ifName;
    int flags = 0;
    int scope = 0;
    int ret = globalinterfacestatecallback.OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceAddedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string iface;
    int ret = globalinterfacestatecallback.OnInterfaceAdded(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceRemovedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string iface;
    int ret = globalinterfacestatecallback.OnInterfaceRemoved(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceChangedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string iface;
    int ret = globalinterfacestatecallback.OnInterfaceChanged(iface, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceLinkStateChangedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string ifName;
    int ret = globalinterfacestatecallback.OnInterfaceLinkStateChanged(ifName, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnRouteChangedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    bool updated = true;
    std::string route;
    std::string gateway;
    std::string ifName;
    int ret = globalinterfacestatecallback.OnRouteChanged(updated, route, gateway, ifName);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnDhcpSuccessTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    NetsysControllerCallback::DhcpResult dhcpResult;
    int ret = globalinterfacestatecallback.OnDhcpSuccess(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnBandwidthReachedLimitTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string limitName;
    std::string iface;
    int ret = globalinterfacestatecallback.OnBandwidthReachedLimit(limitName, iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetIfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    int ret = ethernetService.SetIfaceConfig(DEV_NAME, ic);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(EtherNetServiceTest, GetIfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    std::string iface;
    sptr<InterfaceConfiguration> ifaceConfig = GetIfaceConfig();
    int ret = ethernetService.GetIfaceConfig(iface, ifaceConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(EtherNetServiceTest, IsIfaceActiveTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    int32_t ret = ethernetService.SetIfaceConfig(DEV_NAME, ic);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, GetAllActiveIfacesTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    std::vector<std::string> result;
    int32_t ret = ethernetService.GetAllActiveIfaces(result);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, ResetFactoryTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    int32_t ret = ethernetService.ResetFactory();
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, RegisterIfacesStateChangedTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceStateCallback> callback;
    int32_t ret = ethernetService.RegisterIfacesStateChanged(callback);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, UnregisterIfacesStateChangedTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceStateCallback> callback;
    int32_t ret = ethernetService.UnregisterIfacesStateChanged(callback);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetInterfaceUpTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    int32_t ret = ethernetService.SetInterfaceUp(DEV_NAME);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetInterfaceDownTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    int32_t ret = ethernetService.SetInterfaceDown(DEV_NAME);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, GetInterfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    int32_t ret = ethernetService.GetInterfaceConfig(DEV_NAME, cfg);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetInterfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    std::string deviceId;
    int32_t systemAbilityId = 0;
    ethernetService.OnAddSystemAbility(systemAbilityId, deviceId);
    ethernetService.OnAddSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, deviceId);
    ethernetService.OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, deviceId);

    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = "eth0";
    config.hwAddr = "";
    config.ipv4Addr = "172.17.5.234";
    config.prefixLength = 24;
    config.flags.push_back("up");
    config.flags.push_back("broadcast");
    int32_t ret = ethernetService.SetInterfaceConfig(DEV_NAME, config);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS