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

#include <memory>

#include "route.h"
#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "net_vpn_impl.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
using namespace testing::ext;
}

class NetVpnImplInstance : public NetVpnImpl {
public:
    NetVpnImplInstance(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds);
    int32_t SetUp() override;
    int32_t Destroy() override;
    bool IsInternalVpn() override;
};

class VpnConnStateCbTest : public IVpnConnStateCb {
public:
    VpnConnStateCbTest() = default;
    virtual ~VpnConnStateCbTest() = default;
    void OnVpnConnStateChanged(const VpnConnectState &state) override;
};

NetVpnImplInstance::NetVpnImplInstance(sptr<VpnConfig> config, const std::string &pkg, int32_t userId, std::vector<int32_t> &activeUserIds)
    : NetVpnImpl(config, pkg, userId, activeUserIds)
{
}

int32_t NetVpnImplInstance::SetUp()
{
    return 0;
}

int32_t NetVpnImplInstance::Destroy()
{
    return 0;
}

bool NetVpnImplInstance::IsInternalVpn()
{
    return false;
}

void VpnConnStateCbTest::OnVpnConnStateChanged(const VpnConnectState &state) {}

class NetVpnImplTest : public testing::Test {
public:
    static inline std::unique_ptr<NetVpnImplInstance> netVpnImpl_ = nullptr;
    static void SetUpTestSuite();
};

void NetVpnImplTest::SetUpTestSuite()
{
    sptr<VpnConfig> config = new VpnConfig();
    int32_t userId = 100;
    std::vector<int32_t> activeUserIds;
    netVpnImpl_ = std::make_unique<NetVpnImplInstance>(config, "pkg", userId, activeUserIds);
}

HWTEST_F(NetVpnImplTest, SetUp, TestSize.Level1)
{
    EXPECT_EQ(netVpnImpl_->SetUp(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetVpnImplTest, Destroy, TestSize.Level1)
{
    EXPECT_EQ(netVpnImpl_->Destroy(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetVpnImplTest, IsVpnConnecting, TestSize.Level1)
{
    EXPECT_EQ(netVpnImpl_->IsVpnConnecting(), false);
}

HWTEST_F(NetVpnImplTest, RegisterConnectStateChangedCb001, TestSize.Level1)
{
    std::shared_ptr<VpnConnStateCbTest> callback = nullptr;
    VpnConnectState state = VpnConnectState::VPN_CONNECTED;
    netVpnImpl_->NotifyConnectState(state);
    EXPECT_EQ(netVpnImpl_->RegisterConnectStateChangedCb(callback), NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(NetVpnImplTest, RegisterConnectStateChangedCb002, TestSize.Level1)
{
    std::shared_ptr<VpnConnStateCbTest> callback = std::make_shared<VpnConnStateCbTest>();
    VpnConnectState state = VpnConnectState::VPN_CONNECTED;
    netVpnImpl_->NotifyConnectState(state);
    EXPECT_EQ(netVpnImpl_->RegisterConnectStateChangedCb(callback), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetVpnImplTest, RegisterNetSupplier, TestSize.Level1)
{
    auto& netConnClientIns = NetConnClient::GetInstance();
    netVpnImpl_->netSupplierId_ = 1;
    EXPECT_EQ(netVpnImpl_->RegisterNetSupplier(netConnClientIns), false);
    netVpnImpl_->netSupplierId_ = 0;
    netVpnImpl_->vpnConfig_->isMetered_ = false;
    EXPECT_EQ(netVpnImpl_->RegisterNetSupplier(netConnClientIns), false);
}

HWTEST_F(NetVpnImplTest, UpdateNetSupplierInfo, TestSize.Level1)
{
    auto& netConnClientIns = NetConnClient::GetInstance();
    netVpnImpl_->netSupplierId_ = 0;
    netVpnImpl_->UnregisterNetSupplier(netConnClientIns);
    netVpnImpl_->netSupplierId_ = 1;
    netVpnImpl_->UnregisterNetSupplier(netConnClientIns);
    netVpnImpl_->netSupplierInfo_ = nullptr;
    EXPECT_EQ(netVpnImpl_->UpdateNetSupplierInfo(netConnClientIns, true), false);
    netVpnImpl_->netSupplierId_ = 0;
    EXPECT_EQ(netVpnImpl_->UpdateNetSupplierInfo(netConnClientIns, true), false);
    netVpnImpl_->netSupplierId_ = 1;
    netVpnImpl_->netSupplierInfo_ = new (std::nothrow) NetSupplierInfo();
    EXPECT_EQ(netVpnImpl_->UpdateNetSupplierInfo(netConnClientIns, true), true);
}

HWTEST_F(NetVpnImplTest, UpdateNetLinkInfo, TestSize.Level1)
{
    auto& netConnClientIns = NetConnClient::GetInstance();
    netVpnImpl_->vpnConfig_ = nullptr;
    EXPECT_EQ(netVpnImpl_->UpdateNetLinkInfo(netConnClientIns), false);
    netVpnImpl_->vpnConfig_ = new (std::nothrow) VpnConfig();
    Route route1, toute2;
    netVpnImpl_->vpnConfig_->routes_.push_back(route1);
    netVpnImpl_->vpnConfig_->routes_.push_back(toute2);
    std::string dnsServer1 = "192.168.1.0";
    std::string dnsServer2 = "192.168.2.0";
    netVpnImpl_->vpnConfig_->dnsAddresses_.push_back(dnsServer1);
    netVpnImpl_->vpnConfig_->dnsAddresses_.push_back(dnsServer2);
    std::string domain1 = "baidu.com";
    std::string domain2 = "changhong.com";
    netVpnImpl_->vpnConfig_->searchDomains_.push_back(domain1);
    netVpnImpl_->vpnConfig_->searchDomains_.push_back(domain2);
    EXPECT_EQ(netVpnImpl_->UpdateNetLinkInfo(netConnClientIns), true);
}

HWTEST_F(NetVpnImplTest, GenerateUidRangesByAcceptedApps, TestSize.Level1)
{
    std::set<int32_t> uids = {1, 2, 3};
    std::vector<int32_t> beginUids;
    std::vector<int32_t> endUids;
    netVpnImpl_->GenerateUidRangesByAcceptedApps(uids, beginUids, endUids);
    EXPECT_EQ(beginUids.empty(), false);
}

HWTEST_F(NetVpnImplTest, GenerateUidRangesByRefusedApps, TestSize.Level1)
{
    std::set<int32_t> uids = {1, 2, 3};
    std::vector<int32_t> beginUids;
    std::vector<int32_t> endUids;
    int32_t userId = 0;
    netVpnImpl_->GenerateUidRangesByRefusedApps(userId, uids, beginUids, endUids);
    EXPECT_EQ(beginUids.empty(), false);
}

HWTEST_F(NetVpnImplTest, GetAppsUids, TestSize.Level1)
{
    std::vector<std::string> applications = {"com.baidu.searchbox", "com.quark.browser"};
    int32_t userId = 0;
    std::set<int32_t> uids = netVpnImpl_->GetAppsUids(userId, applications);
    EXPECT_EQ(uids.empty(), true);
}

HWTEST_F(NetVpnImplTest, GenerateUidRanges, TestSize.Level1)
{
    std::vector<int32_t> beginUids;
    std::vector<int32_t> endUids;
    netVpnImpl_->userId_ = AppExecFwk::Constants::INVALID_USERID;
    netVpnImpl_->vpnConfig_->acceptedApplications_ = {"com.baidu.searchbox", "com.quark.browser"};
    int32_t userId = 0;
    netVpnImpl_->vpnConfig_->refusedApplications_ = {"com.qq.reader", "com.tencent.mm"};
    int32_t result = netVpnImpl_->GenerateUidRanges(userId, beginUids, endUids);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetVpnImplTest, ResumeUids001, TestSize.Level1)
{
    int32_t result = netVpnImpl_->ResumeUids();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}
} // namespace NetManagerStandard
} // namespace OHOS
