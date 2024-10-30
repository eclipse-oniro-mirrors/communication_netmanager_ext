/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "net_mgr_log_wrapper.h"
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string TEST_STRING_PERIODDURATION = "M1";
constexpr int32_t WAIT_TIME_SECOND_LONG = 10;
constexpr int32_t TRIGER_DELAY_US = 100000;
constexpr int32_t TEST_CONSTANT_NUM = 3;
constexpr int32_t BACKGROUND_POLICY_TEST_UID = 123;
constexpr uint32_t TEST_UID1 = 10;
constexpr uint32_t TEST_UID2 = 2;
constexpr uint32_t TEST_UID3 = 3;
constexpr uint32_t TEST_UID4 = 4;
constexpr uint32_t TEST_UID5 = 5;
constexpr uint32_t TEST_UID6 = 100;
constexpr uint32_t TEST_UID7 = 1;
constexpr uint32_t TEST_UID8 = 99;
constexpr uint32_t TEST_UID9 = 16;
constexpr uint32_t TEST_WARNING_BYTES = 1234;
constexpr uint32_t TEST_LIMIT_BYTES = 5678;
} // namespace

using namespace testing::ext;
class NetPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void NetPolicyManagerTest::SetUpTestCase()
{
    const std::string tempIccid = "123";
    int32_t ret = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies(tempIccid);
    if (ret == NETMANAGER_SUCCESS) {
        std::cout << "ResetPolicies successful" << std::endl;
    }
}

void NetPolicyManagerTest::TearDownTestCase()
{
    NetManagerBaseAccessToken token;
    int32_t ret1 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID2, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret1" << ret1 << std::endl;
    int32_t ret2 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID3, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret2" << ret2 << std::endl;
    int32_t ret3 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID4, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret3" << ret3 << std::endl;
    int32_t ret4 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID5, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret4" << ret4 << std::endl;
    int32_t ret5 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID6, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret5" << ret5 << std::endl;
    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.networkmatchrule.netType = -1;
    quotaPolicy1.networkmatchrule.simId = std::to_string(TRIGER_DELAY_US);

    NetQuotaPolicy quotaPolicy2;
    quotaPolicy2.networkmatchrule.netType = -1;
    quotaPolicy2.networkmatchrule.simId = "sim_abcdefg_1";

    quotaPolicies.push_back(quotaPolicy1);
    quotaPolicies.push_back(quotaPolicy2);

    int32_t ret6 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
    std::cout << "Result ret6" << ret6 << std::endl;
    int32_t ret7 = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies("sim_abcdefg_1");
    std::cout << "Result ret7" << ret7 << std::endl;
    int32_t ret8 = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies("100000");
    std::cout << "Result ret8" << ret8 << std::endl;
    int32_t ret9 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(true);
    std::cout << "Result ret9" << ret9 << std::endl;
}

void NetPolicyManagerTest::SetUp() {}

void NetPolicyManagerTest::TearDown() {}

sptr<NetPolicyCallbackTest> NetPolicyManagerTest::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callback = new (std::nothrow) NetPolicyCallbackTest();
    return callback;
}

/**
 * @tc.name: NetPolicyManager001
 * @tc.desc: Test NetPolicyManager SetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID7, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    std::cout << "NetPolicyManager001 SetPolicyByUid result:" << result << std::endl;
    ASSERT_EQ(result, NETMANAGER_SUCCESS);

    uint32_t policy = 0;
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetPolicyByUid(TEST_UID7, policy);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
    ASSERT_EQ(policy, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyManager002
 * @tc.desc: Test NetPolicyManager GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager002, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID2, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);

    uint32_t policy = 0;
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetPolicyByUid(TEST_UID2, policy);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
    ASSERT_EQ(policy, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyManager003
 * @tc.desc: Test NetPolicyManager GetUidsByPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager003, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<uint32_t> uids;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND, uids);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager004
 * @tc.desc: Test NetPolicyManager IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager004, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    bool isAllowed = false;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAllowed(TEST_UID7, false, isAllowed);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    ASSERT_TRUE(isAllowed);
}

/**
 * @tc.name: NetPolicyManager005
 * @tc.desc: Test NetPolicyManager IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager005, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    bool isAllowed = false;
    int32_t result =
        DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAllowed(TEST_UID7, std::string("test"), isAllowed);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    ASSERT_TRUE(isAllowed);
}

void TrigerCallback()
{
    usleep(TRIGER_DELAY_US);
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID1, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager006
 * @tc.desc: Test NetPolicyManager RegisterNetPolicyCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager006, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = client->RegisterNetPolicyCallback(callback);
    if (result == NETMANAGER_SUCCESS) {
        ASSERT_EQ(result, NETMANAGER_SUCCESS);
        std::thread trigerCallback(TrigerCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        trigerCallback.join();
        uint32_t uid = callback->GetUid();
        uint32_t netPolicy = callback->GetPolicy();
        std::cout << "NetPolicyManager006 RegisterNetPolicyCallback uid:" << uid
                  << " netPolicy:" << static_cast<uint32_t>(netPolicy) << std::endl;
        ASSERT_EQ(uid, TEST_UID1);
        ASSERT_EQ(netPolicy, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    } else {
        std::cout << "NetPolicyManager006 RegisterNetPolicyCallback return fail" << std::endl;
    }
    NetManagerBaseAccessToken token1;
    result = client->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager007
 * @tc.desc: Test NetPolicyManager SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager007, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = 0;
    quotaPolicy.networkmatchrule.simId = std::to_string(TRIGER_DELAY_US);
    quotaPolicy.quotapolicy.periodStartTime = TRIGER_DELAY_US;
    quotaPolicy.quotapolicy.periodDuration = TEST_STRING_PERIODDURATION;
    quotaPolicy.quotapolicy.warningBytes = TRIGER_DELAY_US;
    quotaPolicy.quotapolicy.limitBytes = TRIGER_DELAY_US;
    quotaPolicy.quotapolicy.lastLimitRemind = -1;
    quotaPolicy.quotapolicy.metered = true;
    quotaPolicy.quotapolicy.source = 0;
    quotaPolicies.push_back(quotaPolicy);
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager008
 * @tc.desc: Test NetPolicyManager GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager008, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager009
 * @tc.desc: Test NetPolicyManager SetCellularPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager009, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy cellularPolicy;
    for (uint32_t i = 0; i < TEST_CONSTANT_NUM; ++i) {
        cellularPolicy.networkmatchrule.simId = std::to_string(i);
        cellularPolicy.quotapolicy.periodStartTime = TRIGER_DELAY_US + i;
        cellularPolicy.quotapolicy.periodDuration = TEST_STRING_PERIODDURATION;
        cellularPolicy.quotapolicy.title = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.quotapolicy.summary = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.quotapolicy.limitBytes = TRIGER_DELAY_US + i;
        cellularPolicy.quotapolicy.limitAction = TEST_CONSTANT_NUM;
        cellularPolicy.quotapolicy.usedBytes = TRIGER_DELAY_US + i;
        cellularPolicy.quotapolicy.usedTimeDuration = TRIGER_DELAY_US + i;
        cellularPolicy.quotapolicy.possessor = std::to_string(TRIGER_DELAY_US + i);

        quotaPolicies.push_back(cellularPolicy);
    }
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager010
 * @tc.desc: Test NetPolicyManager GetCellularPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager010, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager011
 * @tc.desc: Test NetPolicyManager ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager011, TestSize.Level1)
{
    std::string simId = "0";
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies(simId);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0012
 * @tc.desc: Test NetPolicyManager UpdateRemindPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager012, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->UpdateRemindPolicy(
        0, std::to_string(TRIGER_DELAY_US), RemindType::REMIND_TYPE_LIMIT);
    std::cout << "NetPolicyManager012 result value:" << result << std::endl;
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0013
 * @tc.desc: Test NetPolicyManager SetDeviceIdleTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager013, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdleTrustlist({TEST_UID7}, true);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0014
 * @tc.desc: Test NetPolicyManager GetDeviceIdleTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager014, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    std::vector<uint32_t> uids;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetDeviceIdleTrustlist(uids);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0015
 * @tc.desc: Test NetPolicyManager SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager015, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(true);
    ASSERT_EQ(result, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: NetPolicyManager0016
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager016, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    bool backgroundPolicy = false;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicy(backgroundPolicy);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0017
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager017, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(false);
    uint32_t backgroundPolicyOfUid = 0;
    int32_t ret2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(
        BACKGROUND_POLICY_TEST_UID, backgroundPolicyOfUid);
    std::cout << "NetPolicyManager017 GetBackgroundPolicyByUid " << backgroundPolicyOfUid << std::endl;
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(backgroundPolicyOfUid, NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyManager0018
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager018, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(true);
    uint32_t backgroundPolicyOfUid = 0;
    int32_t ret2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(
        BACKGROUND_POLICY_TEST_UID, backgroundPolicyOfUid);
    std::cout << "NetPolicyManager0018 GetBackgroundPolicyByUid " << backgroundPolicyOfUid << std::endl;
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(backgroundPolicyOfUid, NET_BACKGROUND_POLICY_ENABLE);
}

/**
 * @tc.name: NetPolicyManager0019
 * @tc.desc: Test NetPolicyManager SetDeviceIdleTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager019, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdleTrustlist({TEST_UID6}, true);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0020
 * @tc.desc: Test NetPolicyManager SetDeviceIdlePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager020, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    int32_t ret1 = client->SetDeviceIdleTrustlist({TEST_UID8}, true);
    int32_t ret2 = client->SetDeviceIdleTrustlist({TEST_UID6}, true);
    int32_t ret3 = client->SetDeviceIdleTrustlist({TEST_UID3}, true);
    int32_t result = client->SetDeviceIdlePolicy(true);
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret3, NETMANAGER_SUCCESS);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0021
 * @tc.desc: Test NetPolicyManager SetDeviceIdlePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager021, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    int32_t ret1 = client->SetDeviceIdleTrustlist({TEST_UID1}, false);
    int32_t ret2 = client->SetDeviceIdleTrustlist({TEST_UID6}, false);
    int32_t ret3 = client->SetDeviceIdleTrustlist({TEST_UID8}, false);
    int32_t ret4 = client->SetDeviceIdleTrustlist({TEST_UID9}, false);
    int32_t result = client->SetDeviceIdlePolicy(false);
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret3, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret4, NETMANAGER_SUCCESS);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0022
 * @tc.desc: Test NetPolicyManager GetUidsByPolicy.
 * @tc.type: FUNC
 */

HWTEST_F(NetPolicyManagerTest, NetPolicyManager022, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t ret1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID2, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    int32_t ret2 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID3, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    int32_t ret3 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID4, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    int32_t ret4 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID5, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret3, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret4, NETMANAGER_SUCCESS);
    std::vector<uint32_t> uids;
    int32_t ret5 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND, uids);
    ASSERT_EQ(ret5, NETMANAGER_SUCCESS);
    bool result = false;
    bool result2 = false;
    for (const auto &i : uids) {
        if (i == TEST_UID2) {
            result = true;
        }
    }
    for (const auto &i : uids) {
        if (i == TEST_UID3) {
            result2 = true;
        }
    }
    EXPECT_TRUE(result && result2);
    std::vector<uint32_t> uids2;
    int32_t ret6 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(
        NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND, uids2);
    ASSERT_EQ(ret6, NETMANAGER_SUCCESS);
    result = false;
    result2 = false;
    for (const auto &i : uids2) {
        if (i == TEST_UID4) {
            result = true;
        }
    }
    for (const auto &i : uids2) {
        if (i == TEST_UID5) {
            result2 = true;
        }
    }
    EXPECT_TRUE(result && result2);
}

void SetBackgroundPolicyCallback()
{
    NetManagerBaseAccessToken token;
    usleep(TRIGER_DELAY_US);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(false);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
/**
 * @tc.name: NetPolicyManager023
 * @tc.desc: Test NetPolicyManager NetBackgroundPolicyChange.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager023, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = client->RegisterNetPolicyCallback(callback);
    if (result == NETMANAGER_SUCCESS) {
        std::thread setBackgroundPolicyCallback(SetBackgroundPolicyCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setBackgroundPolicyCallback.join();
        bool result2 = callback->GetBackgroundPolicy();
        std::cout << "NetPolicyManager023 Get background policy is:" << result2 << std::endl;
        ASSERT_EQ(result2, false);
    } else {
        std::cout << "NetPolicyManager023 RegisterNetPolicyCallback return fail" << std::endl;
    }
    NetManagerBaseAccessToken token1;
    result = client->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

void SetNetRuleChangeCallback()
{
    NetManagerBaseAccessToken token;
    usleep(TRIGER_DELAY_US);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID6, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager024
 * @tc.desc: Test NetPolicyManager NetMeteredIfacesChange.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager024, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);

    if (result == NETMANAGER_SUCCESS) {
        std::thread setNetRuleChangedCallback(SetNetRuleChangeCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setNetRuleChangedCallback.join();
        uint32_t result2 = callback->GetRule();
        std::cout << "NetPolicyManager024 rule result:" << result2 << std::endl;
        ASSERT_NE(result2, 128);
    } else {
        std::cout << "NetPolicyManager024 SetNetRuleChangeCallback return fail" << std::endl;
    }
    NetManagerBaseAccessToken token1;
    result = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

void SetNetQuotaPoliciesCallback()
{
    NetManagerBaseAccessToken token;
    usleep(TRIGER_DELAY_US);

    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = 0;
    quotaPolicy.networkmatchrule.simId = "sim_abcdefg_1";
    quotaPolicy.quotapolicy.periodDuration = "m2";
    quotaPolicy.quotapolicy.warningBytes = TEST_WARNING_BYTES;
    quotaPolicy.quotapolicy.limitBytes = TEST_LIMIT_BYTES;
    quotaPolicy.quotapolicy.lastLimitRemind = -1;
    quotaPolicy.quotapolicy.metered = true;
    quotaPolicy.quotapolicy.source = 0;
    quotaPolicies.push_back(quotaPolicy);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
/**
 * @tc.name: NetPolicyManager025
 * @tc.desc: Test NetPolicyManager NetQuotaPolicyChange.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager025, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
    if (result == NETMANAGER_SUCCESS) {
        std::thread setNetQuotaPoliciesCallback(SetNetQuotaPoliciesCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setNetQuotaPoliciesCallback.join();
        std::cout << "NetPolicyManager025 result is:" << result << std::endl;
    } else {
        std::cout << "NetPolicyManager025 RegisterNetPolicyCallback return fail" << std::endl;
    }
    NetManagerBaseAccessToken token1;
    result = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager027
 * @tc.desc: Test NetPolicyManager SetPowerSaveTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager027, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    int32_t result1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPowerSaveTrustlist({TEST_UID1}, true);
    ASSERT_EQ(result1, NETMANAGER_SUCCESS);
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPowerSaveTrustlist({TEST_UID6}, true);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
    int32_t result3 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPowerSaveTrustlist({TEST_UID8}, true);
    ASSERT_EQ(result3, NETMANAGER_SUCCESS);
    int32_t result4 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPowerSaveTrustlist({TEST_UID9}, true);
    ASSERT_EQ(result4, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager028
 * @tc.desc: Test NetPolicyManager GetPowerSaveTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager028, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetPowerSaveTrustlist(uids);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager029
 * @tc.desc: Test NetPolicyManager SetNetworkAccessPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager029, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    NetworkAccessPolicy netAccessPolicy;
    netAccessPolicy.wifiAllow = false;
    netAccessPolicy.cellularAllow = false;
    bool reconfirmFlag = true;

    int32_t result1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetworkAccessPolicy(
        TEST_UID1, netAccessPolicy, reconfirmFlag);
    ASSERT_EQ(result1, NETMANAGER_SUCCESS);
    reconfirmFlag = false;
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetworkAccessPolicy(
        TEST_UID6, netAccessPolicy, reconfirmFlag);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager030
 * @tc.desc: Test NetPolicyManager GetNetworkAccessPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager030, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    AccessPolicyParameter parameter;
    parameter.flag = 0;
    parameter.uid = TEST_UID1;
    AccessPolicySave resultSave;

    int32_t result1 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetworkAccessPolicy(parameter, resultSave);
    ASSERT_EQ(result1, NETMANAGER_SUCCESS);
    parameter.flag = 1;
    parameter.uid = TEST_UID6;
    auto result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetworkAccessPolicy(parameter, resultSave);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
