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
#include "extended_vpn_ctl.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#ifdef GTEST_API_
#define private public
#endif
#include "net_manager_constants.h"
#include "networkvpn_service.h"
#include "vpn_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *NET_ACTIVATE_WORK_THREAD = "VPN_CALLBACK_WORK_THREAD";
class VpnEventTestCallback : public VpnEventCallbackStub {
public:
    void OnVpnStateChanged(const bool &isConnected) override{};
    void OnVpnMultiUserSetUp() override{};
};
} // namespace

class NetworkVpnServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = &Singleton<NetworkVpnService>::GetInstance();
    static inline sptr<IVpnEventCallback> eventCallback_ = nullptr;
};

void NetworkVpnServiceTest::SetUpTestCase()
{
    instance_->OnStart();
    eventCallback_ = new (std::nothrow) VpnEventTestCallback();
    ASSERT_NE(eventCallback_, nullptr);
}

void NetworkVpnServiceTest::TearDownTestCase()
{
    instance_->OnStop();
}

void NetworkVpnServiceTest::SetUp() {}

void NetworkVpnServiceTest::TearDown() {}

HWTEST_F(NetworkVpnServiceTest, OnStart, TestSize.Level1)
{
    instance_->state_ = NetworkVpnService::STATE_RUNNING;
    instance_->OnStart();
    instance_->state_ = NetworkVpnService::STATE_STOPPED;
    EXPECT_EQ(instance_->state_, NetworkVpnService::STATE_STOPPED);
}

HWTEST_F(NetworkVpnServiceTest, OnStop, TestSize.Level1)
{
    instance_->policyCallRunner_ = nullptr;
    instance_->OnStop();
    instance_->policyCallRunner_ = AppExecFwk::EventRunner::Create(NET_ACTIVATE_WORK_THREAD);
    instance_->OnStop();
    EXPECT_EQ(instance_->state_, NetworkVpnService::STATE_STOPPED);
}

HWTEST_F(NetworkVpnServiceTest, Dump, TestSize.Level1)
{
    int32_t fd = 1;
    std::vector<std::u16string> args = {};
    EXPECT_EQ(instance_->Dump(fd, args), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, Init, TestSize.Level1)
{
    EXPECT_EQ(instance_->Init(), false);
}

HWTEST_F(NetworkVpnServiceTest, GetDumpMessage, TestSize.Level1)
{
    std::string message;
    instance_->vpnObj_ = nullptr;
    instance_->GetDumpMessage(message);
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId);
    instance_->GetDumpMessage(message);
    EXPECT_EQ(message.empty(), false);
}

HWTEST_F(NetworkVpnServiceTest, Prepare001, TestSize.Level1)
{
    bool isExistVpn = false;
    bool isRun = false;
    std::string pkg;
    EXPECT_EQ(instance_->Prepare(isExistVpn, isRun, pkg), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, SetUpVpn, TestSize.Level1)
{
    int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID;
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    instance_->vpnObj_ = std::make_shared<ExtendedVpnCtl>(config, "", userId);
    EXPECT_EQ(instance_->SetUpVpn(config), NETWORKVPN_ERROR_REFUSE_CREATE_VPN);
}

HWTEST_F(NetworkVpnServiceTest, Protect, TestSize.Level1)
{
    EXPECT_EQ(instance_->Protect(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, DestroyVpn, TestSize.Level1)
{
    instance_->vpnObj_ = nullptr;
    EXPECT_EQ(instance_->DestroyVpn(), NETWORKVPN_ERROR_REFUSE_CREATE_VPN);
}

HWTEST_F(NetworkVpnServiceTest, RegisterSharingEventTest001, TestSize.Level1)
{
    instance_->policyCallRunner_ = AppExecFwk::EventRunner::Create(NET_ACTIVATE_WORK_THREAD);
    instance_->policyCallHandler_ = std::make_shared<AppExecFwk::EventHandler>(instance_->policyCallRunner_);
    EXPECT_EQ(instance_->RegisterVpnEvent(eventCallback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, UnregisterSharingEventTest001, TestSize.Level1)
{
    instance_->policyCallRunner_ = AppExecFwk::EventRunner::Create(NET_ACTIVATE_WORK_THREAD);
    instance_->policyCallHandler_ = std::make_shared<AppExecFwk::EventHandler>(instance_->policyCallRunner_);
    EXPECT_EQ(instance_->UnregisterVpnEvent(eventCallback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, CheckCurrentUser, TestSize.Level1)
{
    int32_t userId = 0;
    EXPECT_EQ(instance_->CheckCurrentAccountType(userId), NETWORKVPN_ERROR_REFUSE_CREATE_VPN);
}

HWTEST_F(NetworkVpnServiceTest, SyncRegisterVpnEvent, TestSize.Level1)
{
    instance_->vpnEventCallbacks_.push_back(eventCallback_);
    EXPECT_EQ(instance_->SyncRegisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    instance_->vpnEventCallbacks_.clear();
    sptr<IVpnEventCallback> eventCallback_1 = new (std::nothrow) VpnEventTestCallback();
    instance_->vpnEventCallbacks_.push_back(eventCallback_1);
    sptr<IVpnEventCallback> eventCallback_2 = new (std::nothrow) VpnEventTestCallback();
    instance_->vpnEventCallbacks_.push_back(eventCallback_2);
    EXPECT_EQ(instance_->SyncRegisterVpnEvent(eventCallback_), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceTest, SyncUnregisterVpnEvent, TestSize.Level1)
{
    EXPECT_EQ(instance_->SyncUnregisterVpnEvent(eventCallback_), NETMANAGER_EXT_SUCCESS);
    instance_->vpnEventCallbacks_.clear();
    EXPECT_EQ(instance_->SyncUnregisterVpnEvent(eventCallback_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}
} // namespace NetManagerStandard
} // namespace OHOS