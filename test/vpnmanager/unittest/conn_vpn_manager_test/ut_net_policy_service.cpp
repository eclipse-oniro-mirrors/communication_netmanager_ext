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

#include <thread>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_policy_firewall.h"
#include "net_policy_rule.h"
#include "net_policy_service.h"
#include "net_policy_traffic.h"
#include "system_ability_definition.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class UtNetPolicyService : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetPolicyService> instance_ = nullptr;
};

void UtNetPolicyService::SetUpTestCase() {}

void UtNetPolicyService::TearDownTestCase() {}

void UtNetPolicyService::SetUp()
{
    instance_ = DelayedSingleton<NetPolicyService>::GetInstance();
    instance_->netPolicyRule_ = std::make_shared<NetPolicyRule>();
    instance_->netPolicyFirewall_ = std::make_shared<NetPolicyFirewall>();
    instance_->netPolicyTraffic_ = std::make_shared<NetPolicyTraffic>();
}

void UtNetPolicyService::TearDown() {}

/**
 * @tc.name: SetPolicyByUid01
 * @tc.desc: Test NetPolicyService SetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetPolicyByUid01, TestSize.Level1)
{
    uint32_t uid = 10000;
    uint32_t policy = 50;
    int32_t ret = instance_->SetPolicyByUid(uid, policy);
    EXPECT_EQ(ret, POLICY_ERR_INVALID_POLICY);
}

/**
 * @tc.name: GetPolicyByUid01
 * @tc.desc: Test NetPolicyService GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetPolicyByUid01, TestSize.Level1)
{
    uint32_t uid = 20000;
    uint32_t policy = 50;
    int32_t ret = instance_->GetPolicyByUid(uid, policy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsUidNetAllowed01
 * @tc.desc: Test NetPolicyService IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, IsUidNetAllowed01, TestSize.Level1)
{
    uint32_t uid = 10000;
    uint32_t policy = 50;
    bool isAllowed = false;
    int32_t ret = instance_->IsUidNetAllowed(uid, policy, isAllowed);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsUidNetAllowed02
 * @tc.desc: Test NetPolicyService IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, IsUidNetAllowed02, TestSize.Level1)
{
    uint32_t uid = 10000;
    std::string ifaceName = "test";
    bool isAllowed = false;
    int32_t ret = instance_->IsUidNetAllowed(uid, ifaceName, isAllowed);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNetQuotaPolicies01
 * @tc.desc: Test NetPolicyService SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNetQuotaPolicies01, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = instance_->SetNetQuotaPolicies(quotaPolicies);
    EXPECT_EQ(ret, POLICY_ERR_INVALID_QUOTA_POLICY);
}

/**
 * @tc.name: GetNetQuotaPolicies01
 * @tc.desc: Test NetPolicyService GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetNetQuotaPolicies01, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = instance_->GetNetQuotaPolicies(quotaPolicies);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetBackgroundPolicy01
 * @tc.desc: Test NetPolicyService SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetBackgroundPolicy01, TestSize.Level1)
{
    instance_->SetBackgroundPolicy(true);
    int32_t ret = instance_->SetBackgroundPolicy(true);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: GetBackgroundPolicy01
 * @tc.desc: Test NetPolicyService GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetBackgroundPolicy01, TestSize.Level1)
{
    bool backgroundPolicy = false;
    int32_t ret = instance_->GetBackgroundPolicy(backgroundPolicy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetBackgroundPolicyByUid01
 * @tc.desc: Test NetPolicyService GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetBackgroundPolicyByUid01, TestSize.Level1)
{
    uint32_t uid = 20000;
    uint32_t backgroundPolicyOfUid = 0;
    int32_t ret = instance_->GetBackgroundPolicyByUid(uid, backgroundPolicyOfUid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetDumpMessage01
 * @tc.desc: Test NetPolicyService GetDumpMessage.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetDumpMessage01, TestSize.Level1)
{
    std::string message;
    int32_t ret = instance_->GetDumpMessage(message);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyServiceBranchTest001
 * @tc.desc: Test NetPolicyService Branch.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyServiceBranchTest001, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    std::string deviceId = "";
    instance_->OnAddSystemAbility(systemAbilityId, deviceId);
    instance_->OnRemoveSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = COMM_NETSYS_NATIVE_SYS_ABILITY_ID;
    instance_->OnAddSystemAbility(systemAbilityId, deviceId);
    instance_->OnRemoveSystemAbility(systemAbilityId, deviceId);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = instance_->Dump(fd, args);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    uint32_t policy = 0;
    std::vector<uint32_t> uids;
    ret = instance_->GetUidsByPolicy(policy, uids);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t netType = 0;
    std::string simId = "";
    uint32_t remindType = 0;
    ret = instance_->UpdateRemindPolicy(netType, simId, remindType);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = instance_->CheckPermission();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->GetDeviceIdleTrustlist(uids);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->SetDeviceIdlePolicy(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_STATUS_EXIST);

    ret = instance_->SetDeviceIdlePolicy(true);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->SetPowerSaveTrustlist(uids, false);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->GetPowerSaveTrustlist(uids);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->SetPowerSavePolicy(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_STATUS_EXIST);

    ret = instance_->SetPowerSavePolicy(true);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

/**
 * @tc.name: NetPolicyServiceBranchTest002
 * @tc.desc: Test NetPolicyService Branch.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyServiceBranchTest002, TestSize.Level1)
{
    ASSERT_TRUE(instance_->netPolicyRule_ != nullptr);
    ASSERT_TRUE(instance_->netPolicyTraffic_ != nullptr);
    ASSERT_TRUE(instance_->netPolicyFirewall_ != nullptr);
    if (instance_->netPolicyFirewall_ == nullptr) {
        return;
    }
    instance_->netPolicyFirewall_->Init();

    uint32_t policy = 0;
    std::vector<uint32_t> uids;
    auto ret = instance_->GetUidsByPolicy(policy, uids);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t netType = 0;
    std::string simId = "";
    uint32_t remindType = 0;
    ret = instance_->UpdateRemindPolicy(netType, simId, remindType);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = instance_->CheckPermission();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->GetDeviceIdleTrustlist(uids);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->SetDeviceIdlePolicy(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_STATUS_EXIST);

    ret = instance_->SetDeviceIdlePolicy(true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->SetPowerSaveTrustlist(uids, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->GetPowerSaveTrustlist(uids);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->SetPowerSavePolicy(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_STATUS_EXIST);

    ret = instance_->SetPowerSavePolicy(true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyServiceBranchTest003
 * @tc.desc: Test NetPolicyService Branch.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyServiceBranchTest003, TestSize.Level1)
{
    instance_->OnStop();
    int32_t netType = 0;
    std::string simId = "";
    uint32_t remindType = 0;
    std::vector<uint32_t> uids;
    auto ret = instance_->UpdateRemindPolicy(netType, simId, remindType);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->CheckPermission();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->GetDeviceIdleTrustlist(uids);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->SetDeviceIdlePolicy(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->SetPowerSaveTrustlist(uids, false);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->GetPowerSaveTrustlist(uids);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->SetPowerSavePolicy(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(UtNetPolicyService, OnRemoveSystemAbility001, TestSize.Level1)
{
    std::string deviceId = "dev1";
    instance_->OnRemoveSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_TRUE(instance_->hasSARemoved_);
}

HWTEST_F(UtNetPolicyService, OnAddSystemAbility001, TestSize.Level1)
{
    std::string deviceId = "dev1";
    instance_->OnAddSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
    EXPECT_FALSE(instance_->hasSARemoved_);
}

/**
 * @tc.name: SetNetworkAccessPolicy01
 * @tc.desc: Test NetPolicyService SetNetworkAccessPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNetworkAccessPolicy01, TestSize.Level1)
{
    int32_t uid = 666;
    NetworkAccessPolicy netAccessPolicy;
    netAccessPolicy.wifiAllow = false;
    netAccessPolicy.cellularAllow = false;
    auto ret = instance_->SetNetworkAccessPolicy(uid, netAccessPolicy, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetworkAccessPolicy01
 * @tc.desc: Test NetPolicyService GetNetworkAccessPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetNetworkAccessPolicy01, TestSize.Level1)
{
    AccessPolicyParameter parameter;
    parameter.flag = 0;
    parameter.uid = 666;
    AccessPolicySave resultSave;
    auto ret = instance_->GetNetworkAccessPolicy(parameter, resultSave);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: DeleteNetworkAccessPolicy01
 * @tc.desc: Test NetPolicyService DeleteNetworkAccessPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, DeleteNetworkAccessPolicy01, TestSize.Level1)
{
    int32_t uid = 666;
    auto ret = instance_->DeleteNetworkAccessPolicy(uid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNicTrafficAllowed001
 * @tc.desc: Test NetPolicyService SetNicTrafficAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNicTrafficAllowed001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<std::string> ifaceName = {"wlan0", "aaa"};
    auto ret = instance_->SetNicTrafficAllowed(ifaceName, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNicTrafficAllowed002
 * @tc.desc: Test NetPolicyService SetNicTrafficAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNicTrafficAllowed002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<std::string> ifaceName = {"wlan0", "aaa"};
    auto ret = instance_->SetNicTrafficAllowed(ifaceName, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNicTrafficAllowed003
 * @tc.desc: Test NetPolicyService SetNicTrafficAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNicTrafficAllowed003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<std::string> ifaceName = {"wlan0"};
    auto ret = instance_->SetNicTrafficAllowed(ifaceName, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNicTrafficAllowed004
 * @tc.desc: Test NetPolicyService SetNicTrafficAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNicTrafficAllowed004, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<std::string> ifaceName = {"wlan0"};
    auto ret = instance_->SetNicTrafficAllowed(ifaceName, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNicTrafficAllowed005
 * @tc.desc: Test NetPolicyService SetNicTrafficAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNicTrafficAllowed005, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<std::string> ifaceName = {};
    auto ret = instance_->SetNicTrafficAllowed(ifaceName, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNicTrafficAllowed006
 * @tc.desc: Test NetPolicyService SetNicTrafficAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNicTrafficAllowed006, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<std::string> ifaceName = {};
    auto ret = instance_->SetNicTrafficAllowed(ifaceName, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS