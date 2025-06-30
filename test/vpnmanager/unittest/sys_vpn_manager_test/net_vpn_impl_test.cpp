/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
    NetVpnImplInstance(sptr<VpnConfig> config, const std::string &pkg, int32_t userId,
        std::vector<int32_t> &activeUserIds);
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

NetVpnImplInstance::NetVpnImplInstance(sptr<VpnConfig> config, const std::string &pkg, int32_t userId,
    std::vector<int32_t> &activeUserIds)
    : NetVpnImpl(config, pkg, userId, activeUserIds)
{}

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

HWTEST_F(NetVpnImplTest, GetConnectedSysVpnConfig001, TestSize.Level1)
{
    sptr<SysVpnConfig> vpnConfig = nullptr;
    EXPECT_EQ(netVpnImpl_->GetConnectedSysVpnConfig(vpnConfig), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetVpnImplTest, NotifyConnectStage001, TestSize.Level1)
{
    std::string stage = "test001";
    int32_t result = 0;
    EXPECT_EQ(netVpnImpl_->NotifyConnectStage(stage, result), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetVpnImplTest, GetSysVpnCertUri001, TestSize.Level1)
{
    int32_t certType = 0;
    std::string certUri;
    EXPECT_EQ(netVpnImpl_->GetSysVpnCertUri(certType, certUri), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetVpnImplTest, GetVpnCertData001, TestSize.Level1)
{
    if (netVpnImpl_ != nullptr) {
        int32_t certType = 0;
        std::vector<int8_t> certData;
        EXPECT_EQ(netVpnImpl_->GetVpnCertData(0, certData), NETMANAGER_EXT_SUCCESS);
    }
}

HWTEST_F(NetVpnImplTest, UpdateNetLinkInfo001, TestSize.Level1)
{
    netVpnImpl_->vpnConfig_ = nullptr;
    EXPECT_EQ(netVpnImpl_->UpdateNetLinkInfo(), false);
    netVpnImpl_->vpnConfig_ = new (std::nothrow) VpnConfig();
    netVpnImpl_->vpnConfig_->isAcceptIPv6_ = true;
    EXPECT_EQ(netVpnImpl_->UpdateNetLinkInfo(), true);
    netVpnImpl_->vpnConfig_->isAcceptIPv6_ = false;
    EXPECT_EQ(netVpnImpl_->UpdateNetLinkInfo(), true);
    netVpnImpl_->SetAllUidRanges();
    netVpnImpl_->vpnConfig_->isAcceptIPv4_ = false;
    netVpnImpl_->vpnConfig_->isAcceptIPv6_ = true;
    std::string dnsServer = "fe80::ea68:19ff:fe63:98bc%7";
    netVpnImpl_->vpnConfig_->dnsAddresses_.push_back(dnsServer);
    EXPECT_EQ(netVpnImpl_->UpdateNetLinkInfo(), true);
}

HWTEST_F(NetVpnImplTest, DelNetLinkInfo001, TestSize.Level1)
{
    auto& netConnClientIns = NetConnClient::GetInstance();
    netVpnImpl_->vpnConfig_ = new (std::nothrow) VpnConfig();
    ASSERT_TRUE(netVpnImpl_->vpnConfig_ != nullptr);
    Route route1;
    netVpnImpl_->vpnConfig_->routes_.push_back(route1);
    netVpnImpl_->vpnConfig_->isAcceptIPv4_ = false;
    netVpnImpl_->AdjustRouteInfo(route1);
    route1.iface_ = "ifaceTest";
    netVpnImpl_->AdjustRouteInfo(route1);
    netVpnImpl_->DelNetLinkInfo(netConnClientIns);
    EXPECT_TRUE(netVpnImpl_->vpnConfig_ != nullptr);
}

HWTEST_F(NetVpnImplTest, GenerateUidRangesByAcceptedApps001, TestSize.Level1)
{
    std::set<int32_t> uids = {1, 2, 5};
    std::vector<int32_t> beginUids;
    std::vector<int32_t> endUids;
    netVpnImpl_->GenerateUidRangesByAcceptedApps(uids, beginUids, endUids);
    EXPECT_FALSE(beginUids.empty());
}

HWTEST_F(NetVpnImplTest, GenerateUidRanges001, TestSize.Level1)
{
    std::vector<int32_t> beginUids;
    std::vector<int32_t> endUids;
    netVpnImpl_->userId_ = AppExecFwk::Constants::INVALID_USERID;
    int32_t userId = AppExecFwk::Constants::INVALID_USERID;
    int32_t result = netVpnImpl_->GenerateUidRanges(userId, beginUids, endUids);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    netVpnImpl_->vpnConfig_->refusedApplications_ = {"com.openharmony.test"};
    result = netVpnImpl_->GenerateUidRanges(userId, beginUids, endUids);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

} // namespace NetManagerStandard
} // namespace OHOS
