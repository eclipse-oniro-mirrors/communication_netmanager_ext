/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <memory>

#include "common_net_conn_callback_test.h"
#include "net_conn_callback_stub.h"
#include "net_manager_constants.h"
#define private public
#include "net_activate.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t TEST_TIMEOUT_MS = 1000;
constexpr uint32_t TEST_REQUEST_ID = 54656;
constexpr const char *TEST_IDENT = "testIdent";

class NetActivateCallbackTest : public INetActivateCallback {
    void OnNetActivateTimeOut(uint32_t reqId) override
    {
        std::cout << "Activate network request " << reqId << " timeout." << std::endl;
    }
};
} // namespace

class NetActivateTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<AppExecFwk::EventRunner> netActEventRunner_ = nullptr;
    static inline std::shared_ptr<AppExecFwk::EventHandler> netActEventHandler_ = nullptr;
    static inline std::shared_ptr<NetActivate> instance_ = nullptr;
    static inline sptr<INetConnCallback> callback_ = nullptr;
    static inline sptr<NetSpecifier> specifier_ = nullptr;
    static inline std::shared_ptr<INetActivateCallback> timeoutCallback_ = nullptr;
};

void NetActivateTest::SetUpTestCase()
{
    callback_ = new (std::nothrow) NetConnCallbackStubCb();
    specifier_ = new (std::nothrow) NetSpecifier();
    timeoutCallback_ = std::make_shared<NetActivateCallbackTest>();
    netActEventRunner_ = AppExecFwk::EventRunner::Create("NET_ACTIVATE_WORK_THREAD");
    netActEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(netActEventRunner_);
    instance_ =
        std::make_shared<NetActivate>(specifier_, callback_, timeoutCallback_, TEST_TIMEOUT_MS, netActEventHandler_);
}

void NetActivateTest::TearDownTestCase()
{
    instance_ = nullptr;
}

void NetActivateTest::SetUp() {}

void NetActivateTest::TearDown() {}

HWTEST_F(NetActivateTest, MatchRequestAndNetworkTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NET_CAPABILITY_INTERNET);
    sptr<NetSupplier> supplier = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps);
    bool ret = instance_->MatchRequestAndNetwork(supplier);
    EXPECT_TRUE(ret);
    sptr<NetSupplier> supplier001 = nullptr;
    ret = instance_->MatchRequestAndNetwork(supplier001);
    EXPECT_FALSE(ret);
    std::string test;
    sptr<NetSupplier> supplier002 = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, test, netCaps);
    ret = instance_->MatchRequestAndNetwork(supplier002);
    EXPECT_TRUE(ret);
    std::set<NetCap> netCaps1;
    netCaps1.insert(NET_CAPABILITY_INTERNET);
    sptr<NetSupplier> supplier003 = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps1);
    ret = instance_->MatchRequestAndNetwork(supplier003);
    EXPECT_TRUE(ret);
    sptr<NetSupplier> supplier004 = new (std::nothrow) NetSupplier(NetBearType::BEARER_CELLULAR, TEST_IDENT, netCaps);
    ret = instance_->MatchRequestAndNetwork(supplier004);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetActivateTest, MatchRequestAndNetworkTest002, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NET_CAPABILITY_INTERNET);
    sptr<NetSupplier> supplier = nullptr;
    bool ret = instance_->MatchRequestAndNetwork(supplier);
    EXPECT_EQ(ret, false);
    supplier = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps);
    ret = instance_->MatchRequestAndNetwork(supplier);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetActivateTest, SetGetRequestIdTest001, TestSize.Level1)
{
    instance_->SetRequestId(TEST_REQUEST_ID);
    uint32_t requestId = instance_->GetRequestId();
    EXPECT_EQ(requestId, TEST_REQUEST_ID);
}

HWTEST_F(NetActivateTest, SetGetServiceSupplyTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps);
    instance_->SetServiceSupply(supplier);
    auto result = instance_->GetServiceSupply();
    EXPECT_EQ(result, supplier);
}

HWTEST_F(NetActivateTest, GetNetCallbackTest001, TestSize.Level1)
{
    auto result = instance_->GetNetCallback();
    EXPECT_EQ(result, callback_);
}

HWTEST_F(NetActivateTest, GetNetSpecifierTest001, TestSize.Level1)
{
    auto result = instance_->GetNetSpecifier();
    EXPECT_EQ(result, specifier_);
}

HWTEST_F(NetActivateTest, CompareByNetworkIdentTest001, TestSize.Level1)
{
    auto result = instance_->GetNetSpecifier();
    EXPECT_EQ(result, specifier_);
}

HWTEST_F(NetActivateTest, CompareByNetworkIdent001, TestSize.Level1)
{
    std::string ident;
    bool ret = instance_->CompareByNetworkIdent(ident, BEARER_DEFAULT, false);
    EXPECT_EQ(ret, true);

    ident = "test1234";
    ret = instance_->CompareByNetworkIdent(ident, BEARER_DEFAULT, false);
    EXPECT_EQ(ret, true);

    instance_->netSpecifier_->ident_ = "test1234";
    ret = instance_->CompareByNetworkIdent(ident, BEARER_DEFAULT, false);
    EXPECT_EQ(ret, true);

    ident = "test5678";
    ret = instance_->CompareByNetworkIdent(ident, BEARER_DEFAULT, false);
    EXPECT_EQ(ret, false);

    instance_->netSpecifier_->ident_ = "wifi";
    ret = instance_->CompareByNetworkIdent(ident, BEARER_DEFAULT, true);
    EXPECT_EQ(ret, true);

    ret = instance_->CompareByNetworkIdent(ident, BEARER_WIFI, true);
    EXPECT_EQ(ret, true);
}

HWTEST_F(NetActivateTest, CompareByNetworkCapabilities001, TestSize.Level1)
{
    NetCaps netCaps;
    netCaps.InsertNetCap(NetCap::NET_CAPABILITY_INTERNET);
    bool ret = instance_->CompareByNetworkCapabilities(netCaps);
    EXPECT_EQ(ret, true);

    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    sptr<NetSpecifier> specifier = nullptr;
    std::weak_ptr<INetActivateCallback> timeoutCallback = std::make_shared<NetActivateCallbackTest>();
    std::shared_ptr<NetActivate> testNetActivate =
        std::make_shared<NetActivate>(specifier, callback, timeoutCallback, TEST_TIMEOUT_MS, netActEventHandler_);

    ret = testNetActivate->CompareByNetworkCapabilities(netCaps);
    EXPECT_EQ(ret, false);
}

HWTEST_F(NetActivateTest, CompareByNetworkNetType001, TestSize.Level1)
{
    NetBearType bearerType = BEARER_WIFI;
    bool ret = instance_->CompareByNetworkNetType(bearerType);
    EXPECT_EQ(ret, true);

    instance_->netSpecifier_->SetType(BEARER_WIFI);
    ret = instance_->CompareByNetworkNetType(bearerType);
    EXPECT_EQ(ret, true);

    bearerType = BEARER_ETHERNET;
    ret = instance_->CompareByNetworkNetType(bearerType);
    EXPECT_EQ(ret, false);

    instance_->netSpecifier_->netCapabilities_.bearerTypes_.clear();
    ret = instance_->CompareByNetworkNetType(bearerType);
    EXPECT_EQ(ret, true);

    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    sptr<NetSpecifier> specifier = nullptr;
    std::weak_ptr<INetActivateCallback> timeoutCallback = std::make_shared<NetActivateCallbackTest>();
    std::shared_ptr<NetActivate> testNetActivate =
        std::make_shared<NetActivate>(specifier, callback, timeoutCallback, TEST_TIMEOUT_MS, netActEventHandler_);
    ret = testNetActivate->CompareByNetworkNetType(bearerType);
    EXPECT_EQ(ret, false);
}

HWTEST_F(NetActivateTest, CompareByNetworkBand001, TestSize.Level1)
{
    uint32_t netLinkUpBand = 100;
    uint32_t netLinkDownBand = 100;
    instance_->netSpecifier_->netCapabilities_.linkUpBandwidthKbps_ = 100;
    instance_->netSpecifier_->netCapabilities_.linkDownBandwidthKbps_ = 100;
    bool ret = instance_->CompareByNetworkBand(netLinkUpBand, netLinkDownBand);
    EXPECT_EQ(ret, true);

    netLinkUpBand = 50;
    ret = instance_->CompareByNetworkBand(netLinkUpBand, netLinkDownBand);
    EXPECT_EQ(ret, false);
}

HWTEST_F(NetActivateTest, HaveCapability001, TestSize.Level1)
{
    instance_->netSpecifier_->netCapabilities_.netCaps_.clear();
    instance_->netSpecifier_->netCapabilities_.netCaps_.insert(NET_CAPABILITY_NOT_VPN);
    NetCap netCap = NET_CAPABILITY_NOT_VPN;
    bool ret = instance_->HaveCapability(netCap);
    EXPECT_EQ(ret, true);

    netCap = NET_CAPABILITY_INTERNET;
    ret = instance_->HaveCapability(netCap);
    EXPECT_EQ(ret, false);

    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    sptr<NetSpecifier> specifier = nullptr;
    std::weak_ptr<INetActivateCallback> timeoutCallback = std::make_shared<NetActivateCallbackTest>();
    std::shared_ptr<NetActivate> testNetActivate =
        std::make_shared<NetActivate>(specifier, callback, timeoutCallback, TEST_TIMEOUT_MS, netActEventHandler_);

    ret = testNetActivate->HaveCapability(netCap);
    EXPECT_EQ(ret, false);
}

HWTEST_F(NetActivateTest, HaveTypes001, TestSize.Level1)
{
    std::set<NetBearType> bearerTypes;
    bool ret = instance_->HaveTypes(bearerTypes);
    EXPECT_EQ(ret, false);

    bearerTypes.insert(BEARER_WIFI);
    instance_->netSpecifier_->netCapabilities_.bearerTypes_.insert(BEARER_VPN);
    ret = instance_->HaveTypes(bearerTypes);
    EXPECT_EQ(ret, false);

    instance_->netSpecifier_->netCapabilities_.bearerTypes_.insert(BEARER_WIFI);
    ret = instance_->HaveTypes(bearerTypes);
    EXPECT_EQ(ret, true);

    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    sptr<NetSpecifier> specifier = nullptr;
    std::weak_ptr<INetActivateCallback> timeoutCallback = std::make_shared<NetActivateCallbackTest>();
    std::shared_ptr<NetActivate> testNetActivate =
        std::make_shared<NetActivate>(specifier, callback, timeoutCallback, TEST_TIMEOUT_MS, netActEventHandler_);

    ret = testNetActivate->HaveTypes(bearerTypes);
    EXPECT_EQ(ret, false);
}

HWTEST_F(NetActivateTest, NetActivateBranchTest001, TestSize.Level1)
{
    instance_->netConnCallback_ = nullptr;
    instance_->TimeOutNetAvailable();
    EXPECT_TRUE(instance_->GetNetCallback() == nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS