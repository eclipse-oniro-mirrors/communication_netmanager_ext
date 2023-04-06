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

class AccessToken {
public:
    explicit AccessToken(HapPolicyParams &testPolicyPrams) : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(tokenIdEx.tokenIDEx);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};

class EtherNetServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    sptr<InterfaceConfiguration> GetIfaceConfig();
    void SetUp();
    void TearDown();
};

sptr<InterfaceConfiguration> EtherNetServiceProxyTest::GetIfaceConfig()
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

void EtherNetServiceProxyTest::SetUpTestCase() {}

void EtherNetServiceProxyTest::TearDownTestCase() {}

void EtherNetServiceProxyTest::SetUp() {}

void EtherNetServiceProxyTest::TearDown() {}

HWTEST_F(EtherNetServiceProxyTest, SetIfaceConfigTest001, TestSize.Level1)
{
    AccessToken accessToken(testPolicyPrams2);
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ethernetServiceProxy.SetIfaceConfig(DEV_NAME, ic);
}

HWTEST_F(EtherNetServiceProxyTest, GetIfaceConfigTest001, TestSize.Level1)
{
    AccessToken accessToken(testPolicyPrams1);
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    sptr<InterfaceConfiguration> ifaceConfig = new (std::nothrow) InterfaceConfiguration();
    ethernetServiceProxy.GetIfaceConfig(DEV_NAME, ifaceConfig);
}

HWTEST_F(EtherNetServiceProxyTest, IsIfaceActiveTest001, TestSize.Level1)
{
    AccessToken accessToken(testPolicyPrams1);
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    sptr<InterfaceConfiguration> ifaceConfig = new (std::nothrow) InterfaceConfiguration();
    std::string ifcaeName = "eth0";
    int32_t activeStatus = -1;
    ethernetServiceProxy.IsIfaceActive(ifcaeName, activeStatus);
}

HWTEST_F(EtherNetServiceProxyTest, GetAllActiveIfacesTest001, TestSize.Level1)
{
    AccessToken accessToken(testPolicyPrams1);
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    std::vector<std::string> result;
    ethernetServiceProxy.GetAllActiveIfaces(result);
}

HWTEST_F(EtherNetServiceProxyTest, ResetFactoryTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    ethernetServiceProxy.ResetFactory();
}

HWTEST_F(EtherNetServiceProxyTest, SetInterfaceUpTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    AccessToken accessToken(testPolicyPrams2);
    ethernetServiceProxy.SetInterfaceUp(DEV_NAME);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ethernetServiceProxy.GetInterfaceConfig(DEV_NAME, cfg);
}

HWTEST_F(EtherNetServiceProxyTest, SetInterfaceDownTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    AccessToken accessToken(testPolicyPrams2);
    ethernetServiceProxy.SetInterfaceDown(DEV_NAME);
}

HWTEST_F(EtherNetServiceProxyTest, SetInterfaceConfigTest001, TestSize.Level1)
{
    EthernetServiceProxy ethernetServiceProxy(nullptr);
    AccessToken accessToken(testPolicyPrams2);
    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = "eth0";
    config.hwAddr = "";
    config.ipv4Addr = "172.17.5.234";
    config.prefixLength = 24;
    config.flags.push_back("up");
    config.flags.push_back("broadcast");
    ethernetServiceProxy.SetInterfaceConfig(DEV_NAME, config);
}
} // namespace NetManagerStandard
} // namespace OHOS