/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "inet_addr.h"
#include "interface_configuration.h"
#include "interface_type.h"
#include "nativetoken_kit.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "singleton.h"
#include "static_configuration.h"
#include "token_setproc.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
constexpr const char *CONNECTIVITY_INTERNAL = "ohos.permission.CONNECTIVITY_INTERNAL";
constexpr const char *GET_NETWORK_INFO = "ohos.permission.GET_NETWORK_INFO";
constexpr const char *BUNDLENAME = "ethernet_manager_test";
constexpr const char *DEV_NAME = "eth0";
constexpr const char *DEV_UP = "up";
constexpr const char *DEV_DOWN = "down";
} // namespace

class EthernetManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<InterfaceConfiguration> GetIfaceConfig();
    bool CheckIfaceUp(const std::string &iface);
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

void GrantPermission(const std::string &appId, std::string permissionName)
{
    HapInfoParams hapInfoParams = {
        .userID = 1, .bundleName = appId, .instIndex = 0, .appIDDesc = "app need sync permission"};
    PermissionDef permissionDef = {.permissionName = permissionName,
                                   .bundleName = appId,
                                   .grantMode = 1,
                                   .availableLevel = ATokenAplEnum::APL_SYSTEM_BASIC,
                                   .label = "label",
                                   .labelId = 1,
                                   .description = "permission define",
                                   .descriptionId = 1};
    PermissionStateFull permissionStateFull = {.permissionName = permissionName,
                                               .isGeneral = true,
                                               .resDeviceID = {"local"},
                                               .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                               .grantFlags = {1}};
    HapPolicyParams hapPolicyParams = {.apl = ATokenAplEnum::APL_SYSTEM_BASIC,
                                       .domain = "test.domain",
                                       .permList = {permissionDef},
                                       .permStateList = {permissionStateFull}};
    AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(hapInfoParams, hapPolicyParams);
    if (tokenIdEx.tokenIdExStruct.tokenID == 0) {
        unsigned int tokenIdOld = 0;
        tokenIdOld =
            AccessTokenKit::GetHapTokenID(hapInfoParams.userID, hapInfoParams.bundleName, hapInfoParams.instIndex);
        if (tokenIdOld == 0) {
            return;
        }
        int32_t ret = AccessTokenKit::DeleteToken(tokenIdOld);
        if (ret != 0) {
            return;
        }
        tokenIdEx = AccessTokenKit::AllocHapToken(hapInfoParams, hapPolicyParams);
        if (tokenIdEx.tokenIdExStruct.tokenID == 0) {
            return;
        }
    }
    SetSelfTokenID(tokenIdEx.tokenIdExStruct.tokenID);
    AccessTokenKit::GrantPermission(tokenIdEx.tokenIdExStruct.tokenID, permissionName, PERMISSION_USER_FIXED);
}

bool EthernetManagerTest::CheckIfaceUp(const std::string &iface)
{
    GrantPermission(BUNDLENAME, GET_NETWORK_INFO);
    return DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(iface) == 1;
}

/**
 * @tc.name: EthernetManager001
 * @tc.desc: Test EthernetManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager001, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    GrantPermission(BUNDLENAME, CONNECTIVITY_INTERNAL);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ASSERT_EQ(DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(DEV_NAME, ic), 0);
}

/**
 * @tc.name: EthernetManager002
 * @tc.desc: Test EthernetManager GetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager002, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    GrantPermission(BUNDLENAME, GET_NETWORK_INFO);
    sptr<InterfaceConfiguration> ic = DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(DEV_NAME);
    ASSERT_TRUE(ic != nullptr);
}

/**
 * @tc.name: EthernetManager003
 * @tc.desc: Test EthernetManager IsIfaceActive.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager003, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    GrantPermission(BUNDLENAME, GET_NETWORK_INFO);
    ASSERT_EQ(DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(DEV_NAME), 1);
}

/**
 * @tc.name: EthernetManager004
 * @tc.desc: Test EthernetManager GetAllActiveIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager004, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    GrantPermission(BUNDLENAME, GET_NETWORK_INFO);
    std::vector<std::string> result = DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces();
    std::vector<std::string>::iterator it = std::find(result.begin(), result.end(), DEV_NAME);
    ASSERT_TRUE(it != result.end());
}

/**
 * @tc.name: EthernetManager005
 * @tc.desc: Test EthernetManager ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager005, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    ASSERT_FALSE(DelayedSingleton<EthernetClient>::GetInstance()->ResetFactory() > 0);
}

HWTEST_F(EthernetManagerTest, EthernetManager006, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ASSERT_TRUE(DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(DEV_NAME, cfg));
    ASSERT_FALSE(cfg.ifName.empty());
    ASSERT_FALSE(cfg.hwAddr.empty());
}

HWTEST_F(EthernetManagerTest, EthernetManager007, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceDown(DEV_NAME);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ASSERT_TRUE(DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(DEV_NAME, cfg));
    auto fit = std::find(cfg.flags.begin(), cfg.flags.end(), DEV_DOWN);
    ASSERT_EQ(cfg.ifName, DEV_NAME);
    ASSERT_TRUE(*fit == DEV_DOWN);
    ASSERT_TRUE(result == 0);
}

HWTEST_F(EthernetManagerTest, EthernetManager008, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceUp(DEV_NAME);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    ASSERT_TRUE(DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(DEV_NAME, cfg));
    auto fit = std::find(cfg.flags.begin(), cfg.flags.end(), DEV_UP);
    ASSERT_EQ(cfg.ifName, DEV_NAME);
    ASSERT_TRUE(*fit == DEV_UP);
    ASSERT_TRUE(result == 0);
}
} // namespace NetManagerStandard
} // namespace OHOS