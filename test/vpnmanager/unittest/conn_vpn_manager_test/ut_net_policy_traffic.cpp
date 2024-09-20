/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "net_manager_constants.h"
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_policy_inner_define.h"
#include "net_policy_traffic.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t NO_DATA_SIZE = 0;
const std::string TEST_STRING_PERIODDURATION = "M1";
const std::string ICCID_1 = "sim_abcdefg_1";
const std::string ICCID_2 = "sim_abcdefg_2";
constexpr uint32_t TEST_WARNING_BYTES_1 = 321;
constexpr uint32_t TEST_WARNING_BYTES_2 = 123;
constexpr uint32_t TEST_WARNING_BYTES_3 = 123456;
constexpr uint32_t TEST_LIMIT_BYTES_1 = 4321;
constexpr uint32_t TEST_LIMIT_BYTES_2 = 1234;
constexpr uint32_t TEST_LIMIT_BYTES_3 = 1234567;
constexpr uint32_t TEST_LAST_WARNING_REMIND_1 = 7654321;
constexpr uint32_t TEST_LAST_WARNING_REMIND_2 = 1234567;
constexpr uint32_t TEST_LAST_LIMIT_REMIND_1 = 87654321;
constexpr uint32_t TEST_LAST_LIMIT_REMIND_2 = 12345678;

std::shared_ptr<NetPolicyTraffic> g_netPolicyTraffic = nullptr;

using namespace testing::ext;
class UtNetPolicyTraffic : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void UtNetPolicyTraffic::SetUpTestCase()
{
    g_netPolicyTraffic = std::make_shared<NetPolicyTraffic>();
    g_netPolicyTraffic->Init();
}

void UtNetPolicyTraffic::TearDownTestCase()
{
    g_netPolicyTraffic.reset();
}

void UtNetPolicyTraffic::SetUp()
{
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.networkmatchrule.simId = ICCID_1;
    quotaPolicy1.quotapolicy.periodDuration = "M1";
    quotaPolicy1.networkmatchrule.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy1.quotapolicy.warningBytes = TEST_WARNING_BYTES_1;
    quotaPolicy1.quotapolicy.limitBytes = TEST_LIMIT_BYTES_1;
    quotaPolicy1.quotapolicy.lastWarningRemind = TEST_LAST_WARNING_REMIND_1;
    quotaPolicy1.quotapolicy.lastLimitRemind = TEST_LAST_LIMIT_REMIND_1;
    quotaPolicy1.quotapolicy.metered = true;
    quotaPolicy1.quotapolicy.limitAction = LimitAction::LIMIT_ACTION_ALERT_ONLY;

    NetQuotaPolicy quotaPolicy2;
    quotaPolicy2.networkmatchrule.simId = ICCID_2;
    quotaPolicy2.quotapolicy.periodDuration = "Y1";
    quotaPolicy2.networkmatchrule.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy2.quotapolicy.warningBytes = TEST_WARNING_BYTES_2;
    quotaPolicy2.quotapolicy.limitBytes = TEST_LIMIT_BYTES_2;
    quotaPolicy2.quotapolicy.lastWarningRemind = TEST_LAST_WARNING_REMIND_2;
    quotaPolicy2.quotapolicy.lastLimitRemind = TEST_LAST_LIMIT_REMIND_2;
    quotaPolicy2.quotapolicy.metered = true;
    quotaPolicy2.quotapolicy.limitAction = LimitAction::LIMIT_ACTION_ACCESS_DISABLED;

    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    quotaPolicies.push_back(quotaPolicy2);
    g_netPolicyTraffic->UpdateQuotaPolicies(quotaPolicies);
}

void UtNetPolicyTraffic::TearDown()
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    g_netPolicyTraffic->UpdateQuotaPolicies(quotaPolicies);
}

/**
 * @tc.name: UpdateQuotaPolicies001
 * @tc.desc: Test NetPolicyTraffic UpdateQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, UpdateQuotaPolicies001, TestSize.Level1)
{
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.networkmatchrule.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy1.networkmatchrule.simId = ICCID_1;
    quotaPolicy1.quotapolicy.periodDuration = "M1";
    quotaPolicy1.quotapolicy.warningBytes = TEST_WARNING_BYTES_3;
    quotaPolicy1.quotapolicy.limitBytes = TEST_LIMIT_BYTES_3;
    quotaPolicy1.quotapolicy.lastLimitRemind = -1;
    quotaPolicy1.quotapolicy.metered = 0;
    quotaPolicy1.quotapolicy.source = 0;

    NetQuotaPolicy quotaPolicy2;
    quotaPolicy2.networkmatchrule.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy2.networkmatchrule.simId = ICCID_2;
    quotaPolicy2.quotapolicy.periodDuration = "Y1";
    quotaPolicy2.quotapolicy.warningBytes = TEST_WARNING_BYTES_3;
    quotaPolicy2.quotapolicy.limitBytes = TEST_LIMIT_BYTES_3;
    quotaPolicy2.quotapolicy.lastLimitRemind = -1;
    quotaPolicy2.quotapolicy.metered = 0;
    quotaPolicy2.quotapolicy.source = 0;
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    quotaPolicies.push_back(quotaPolicy2);
    int32_t result = g_netPolicyTraffic->UpdateQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetQuotaPolicies001
 * @tc.desc: Test NetPolicyTraffic GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, GetNetQuotaPolicies001, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t result = g_netPolicyTraffic->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    ASSERT_GT(quotaPolicies.size(), NO_DATA_SIZE);
}

/**
 * @tc.name: UpdateRemindPolicy001
 * @tc.desc: Test NetPolicyTraffic UpdateRemindPolicy REMIND_TYPE_LIMIT
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, UpdateRemindPolicy001, TestSize.Level1)
{
    int32_t result =
        g_netPolicyTraffic->UpdateRemindPolicy(NetBearType::BEARER_CELLULAR, ICCID_1, RemindType::REMIND_TYPE_LIMIT);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    std::vector<NetQuotaPolicy> quotaPolicies;
    result = g_netPolicyTraffic->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    for (auto &quotaPolicy : quotaPolicies) {
        if (quotaPolicy.networkmatchrule.netType == NetBearType::BEARER_CELLULAR &&
            quotaPolicy.networkmatchrule.simId == ICCID_1) {
            if (quotaPolicy.quotapolicy.lastLimitRemind < 0) {
                break;
            }
            auto now = time(nullptr);
            ASSERT_LT(now - quotaPolicy.quotapolicy.lastLimitRemind, 100);
            return;
        }
    }
    FAIL();
}

/**
 * @tc.name: UpdateRemindPolicy002
 * @tc.desc: Test NetPolicyTraffic UpdateRemindPolicy REMIND_TYPE_WARNING
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, UpdateRemindPolicy002, TestSize.Level1)
{
    int32_t result =
        g_netPolicyTraffic->UpdateRemindPolicy(NetBearType::BEARER_CELLULAR, ICCID_2, RemindType::REMIND_TYPE_WARNING);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    std::vector<NetQuotaPolicy> quotaPolicies;
    result = g_netPolicyTraffic->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    for (auto &quotaPolicy : quotaPolicies) {
        if (quotaPolicy.networkmatchrule.netType == NetBearType::BEARER_CELLULAR &&
            quotaPolicy.networkmatchrule.simId == ICCID_2) {
            if (quotaPolicy.quotapolicy.lastWarningRemind < 0) {
                break;
            }
            auto now = time(nullptr);
            ASSERT_LT(now - quotaPolicy.quotapolicy.lastWarningRemind, 100);
            return;
        }
    }
    FAIL();
}

/**
 * @tc.name: UpdateRemindPolicy003
 * @tc.desc: Test NetPolicyTraffic UpdateRemindPolicy No RemindType
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, UpdateRemindPolicy003, TestSize.Level1)
{
    uint32_t errorRemindType = 3;
    int32_t result = g_netPolicyTraffic->UpdateRemindPolicy(NetBearType::BEARER_CELLULAR, ICCID_2, errorRemindType);
    ASSERT_EQ(result, NETMANAGER_ERR_PARAMETER_ERROR);
    std::vector<NetQuotaPolicy> quotaPolicies;
    result = g_netPolicyTraffic->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    for (auto &quotaPolicy : quotaPolicies) {
        if (quotaPolicy.networkmatchrule.netType == NetBearType::BEARER_CELLULAR &&
            quotaPolicy.networkmatchrule.simId == ICCID_2) {
            if (quotaPolicy.quotapolicy.lastWarningRemind < 0) {
                break;
            }
            auto now = time(nullptr);
            ASSERT_GT(now - quotaPolicy.quotapolicy.lastWarningRemind, 100);
            return;
        }
    }
    FAIL();
}

/**
 * @tc.name: HandleEvent001
 * @tc.desc: Test NetPolicyTraffic HandleEvent
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, HandleEvent001, TestSize.Level1)
{
    int32_t eventId = 0;
    std::shared_ptr<PolicyEvent> policyEvent = nullptr;
    g_netPolicyTraffic->HandleEvent(eventId, policyEvent);

    // Test NetPolicyTraffic GetMeteredIfaces
    auto &ifaces = g_netPolicyTraffic->GetMeteredIfaces();
    ASSERT_GE(ifaces.size(), NO_DATA_SIZE);
}

/**
 * @tc.name: ResetPolicies001
 * @tc.desc: Test NetPolicyTraffic ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, ResetPolicies001, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = g_netPolicyTraffic->ResetPolicies(ICCID_1);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    g_netPolicyTraffic->GetNetQuotaPolicies(quotaPolicies);
    for (auto quotaPolicy : quotaPolicies) {
        if (quotaPolicy.networkmatchrule.simId == ICCID_1) {
            if (quotaPolicy.quotapolicy.periodDuration == "M1" &&
                quotaPolicy.quotapolicy.warningBytes == DATA_USAGE_UNKNOWN &&
                quotaPolicy.quotapolicy.limitBytes == DATA_USAGE_UNKNOWN &&
                quotaPolicy.quotapolicy.lastWarningRemind == REMIND_NEVER &&
                quotaPolicy.quotapolicy.lastLimitRemind == REMIND_NEVER && !quotaPolicy.quotapolicy.metered) {
                SUCCEED();
                return;
            }
        }
    }
    FAIL();
}


/**
 * @tc.name: ResetPolicies002
 * @tc.desc: Test NetPolicyTraffic ResetPolicies
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, ResetPolicies002, TestSize.Level1)
{
    auto ret = g_netPolicyTraffic->ResetPolicies();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: ReachedLimit001
 * @tc.desc: Test NetPolicyTraffic ReachedLimit
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, ReachedLimit001, TestSize.Level1)
{
    std::string testIface = "wlan0";
    g_netPolicyTraffic->ReachedLimit(testIface);

    // Test NetPolicyTraffic GetDumpMessage
    std::string msg;
    g_netPolicyTraffic->GetDumpMessage(msg);
    ASSERT_FALSE(msg.empty());
}

/**
 * @tc.name: UpdateQuotaPoliciesInner001
 * @tc.desc: Test NetPolicyTraffic UpdateQuotaPoliciesInner
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, UpdateQuotaPoliciesInner001, TestSize.Level1)
{
    auto ret = g_netPolicyTraffic->UpdateQuotaPoliciesInner();
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetQuotaRemain001
 * @tc.desc: Test NetPolicyTraffic GetQuotaRemain
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, GetQuotaRemain001, TestSize.Level1)
{
    // Test NetPolicyTraffic UpdateQuotaNotify
    g_netPolicyTraffic->UpdateQuotaNotify();

    NetQuotaPolicy quotaPolicy;
    auto ret = g_netPolicyTraffic->GetQuotaRemain(quotaPolicy);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: UpdateNetEnableStatus001
 * @tc.desc: Test NetPolicyTraffic UpdateNetEnableStatus
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, UpdateNetEnableStatus001, TestSize.Level1)
{
    // Test NetPolicyTraffic UpdateNetEnableStatus
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.quotapolicy.metered = true;
    g_netPolicyTraffic->UpdateNetEnableStatus(quotaPolicy);

    quotaPolicy.quotapolicy.metered = false;
    quotaPolicy.quotapolicy.limitAction = LIMIT_ACTION_ACCESS_DISABLED;
    g_netPolicyTraffic->UpdateNetEnableStatus(quotaPolicy);

    quotaPolicy.quotapolicy.limitAction = LIMIT_ACTION_NONE;
    g_netPolicyTraffic->UpdateNetEnableStatus(quotaPolicy);

    // Test NetPolicyTraffic IsValidQuotaPolicy
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_DEFAULT;
    auto ret = g_netPolicyTraffic->IsValidQuotaPolicy(quotaPolicy);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: FormalizeQuotaPolicies001
 * @tc.desc: Test NetPolicyTraffic FormalizeQuotaPolicies
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, FormalizeQuotaPolicies001, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.quotapolicy.metered = false;
    quotaPolicy.quotapolicy.limitAction = LIMIT_ACTION_NONE;
    quotaPolicy.quotapolicy.limitBytes = DATA_USAGE_UNKNOWN;
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_DEFAULT;
    quotaPolicy.quotapolicy.periodDuration = "ff";
    quotaPolicies.push_back(quotaPolicy);
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy.quotapolicy.periodDuration = "";
    quotaPolicies.push_back(quotaPolicy);
    quotaPolicy.quotapolicy.periodDuration = "testPeriodDuration";
    quotaPolicies.push_back(quotaPolicy);
    quotaPolicy.quotapolicy.periodDuration = "M1";
    quotaPolicies.push_back(quotaPolicy);
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_WIFI;
    quotaPolicies.push_back(quotaPolicy);
    quotaPolicy.quotapolicy.limitBytes = DATA_USAGE_UNLIMITED;
    quotaPolicy.quotapolicy.warningBytes = DATA_USAGE_UNLIMITED;
    quotaPolicies.push_back(quotaPolicy);
    g_netPolicyTraffic->FormalizeQuotaPolicies(quotaPolicies);

    // Test NetPolicyTraffic UpdateMeteredIfacesQuota
    auto ret = g_netPolicyTraffic->UpdateMeteredIfacesQuota();
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: IsValidQuotaPolicy002
 * @tc.desc: Test NetPolicyTraffic IsValidQuotaPolicy
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidQuotaPolicy002, TestSize.Level1)
{
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_BLUETOOTH;
    quotaPolicy.quotapolicy.periodDuration = "";
    auto ret = g_netPolicyTraffic->IsValidQuotaPolicy(quotaPolicy);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidQuotaPolicy003
 * @tc.desc: Test NetPolicyTraffic IsValidQuotaPolicy
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidQuotaPolicy003, TestSize.Level1)
{
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_BLUETOOTH;
    quotaPolicy.quotapolicy.periodDuration = "M1";
    auto ret = g_netPolicyTraffic->IsValidQuotaPolicy(quotaPolicy);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetTotalQuota001
 * @tc.desc: Test NetPolicyTraffic GetTotalQuota
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, GetTotalQuota001, TestSize.Level1)
{
    // Test NetPolicyTraffic SetNetworkEnableStatus
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_BLUETOOTH;
    quotaPolicy.quotapolicy.periodDuration = "M1";
    g_netPolicyTraffic->SetNetworkEnableStatus(quotaPolicy, true);

    // Test NetPolicyTraffic NotifyQuotaWarning
    int64_t testQuota = 23;
    g_netPolicyTraffic->NotifyQuotaWarning(testQuota);

    // Test NetPolicyTraffic NotifyQuotaLimit
    g_netPolicyTraffic->NotifyQuotaLimitReminded(testQuota);

    // Test NetPolicyTraffic GetTotalQuota
    auto ret = g_netPolicyTraffic->GetTotalQuota(quotaPolicy);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: WriteQuotaPolicies001
 * @tc.desc: Test NetPolicyTraffic WriteQuotaPolicies
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, WriteQuotaPolicies001, TestSize.Level1)
{
    // Test NetPolicyTraffic PublishQuotaEvent
    std::string action = "testAction";
    std::string describe = "testDescribe";
    int64_t quota = 1;
    g_netPolicyTraffic->PublishQuotaEvent(action, describe, quota);

    auto ret = g_netPolicyTraffic->WriteQuotaPolicies();
    g_netPolicyTraffic->ReadQuotaPolicies();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetMatchIfaces001
 * @tc.desc: Test NetPolicyTraffic GetMatchIfaces
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, GetMatchIfaces001, TestSize.Level1)
{
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_ETHERNET;
    auto ret = g_netPolicyTraffic->GetMatchIfaces(quotaPolicy);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetMatchIfaces002
 * @tc.desc: Test NetPolicyTraffic GetMatchIfaces
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, GetMatchIfaces002, TestSize.Level1)
{
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_CELLULAR;
    auto ret = g_netPolicyTraffic->GetMatchIfaces(quotaPolicy);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetMatchIfaces003
 * @tc.desc: Test NetPolicyTraffic GetMatchIfaces
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, GetMatchIfaces003, TestSize.Level1)
{
    NetQuotaPolicy quotaPolicy;
    quotaPolicy.networkmatchrule.netType = NetBearType::BEARER_WIFI;
    auto ret = g_netPolicyTraffic->GetMatchIfaces(quotaPolicy);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: IsValidNetType001
 * @tc.desc: Test NetPolicyTraffic IsValidNetType
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidNetType001, TestSize.Level1)
{
    const std::vector<NetBearType> list = {BEARER_CELLULAR, BEARER_WIFI, BEARER_BLUETOOTH,
                                           BEARER_ETHERNET, BEARER_VPN,  BEARER_WIFI_AWARE};
    std::for_each(list.begin(), list.end(), [](const auto &it) {
        auto ret = g_netPolicyTraffic->IsValidNetType(it);
        EXPECT_TRUE(ret);
    });
    auto ret = g_netPolicyTraffic->IsValidNetType(BEARER_DEFAULT);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration001
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration001, TestSize.Level1)
{
    const std::string periodDuration = "";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration002
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration002, TestSize.Level1)
{
    const std::string periodDuration = "M";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration003
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration003, TestSize.Level1)
{
    const std::string periodDuration = "D0";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration004
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration004, TestSize.Level1)
{
    const std::string periodDuration = "D40";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration005
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration005, TestSize.Level1)
{
    const std::string periodDuration = "M0";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration006
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration006, TestSize.Level1)
{
    const std::string periodDuration = "M13";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration007
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration007, TestSize.Level1)
{
    const std::string periodDuration = "Y0";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration008
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration008, TestSize.Level1)
{
    const std::string periodDuration = "Y400";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration009
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration009, TestSize.Level1)
{
    const std::string periodDuration = "D15";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration010
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration010, TestSize.Level1)
{
    const std::string periodDuration = "M4";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration011
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration011, TestSize.Level1)
{
    const std::string periodDuration = "Y10";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsValidPeriodDuration012
 * @tc.desc: Test NetPolicyTraffic IsValidPeriodDuration
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, IsValidPeriodDuration012, TestSize.Level1)
{
    const std::string periodDuration = "S1";
    auto ret = g_netPolicyTraffic->IsValidPeriodDuration(periodDuration);
    EXPECT_FALSE(ret);
}
} // namespace NetManagerStandard
} // namespace OHOS
