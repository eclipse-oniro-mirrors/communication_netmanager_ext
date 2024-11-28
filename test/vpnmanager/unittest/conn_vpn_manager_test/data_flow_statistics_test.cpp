/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <ctime>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "data_flow_statistics.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_callback_test.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t TEST_UID = 1001;
const std::string ETH_IFACE_NAME = "eth0";
} // namespace
class DataFlowStatisticsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetStatsCallbackTest> GetINetStatsCallbackSample() const;
};

void DataFlowStatisticsTest::SetUpTestCase() {}

void DataFlowStatisticsTest::TearDownTestCase() {}

void DataFlowStatisticsTest::SetUp() {}

void DataFlowStatisticsTest::TearDown() {}

sptr<NetStatsCallbackTest> DataFlowStatisticsTest::GetINetStatsCallbackSample() const
{
    sptr<NetStatsCallbackTest> callback = new (std::nothrow) NetStatsCallbackTest();
    return callback;
}

/**
 * @tc.name: NetStatsManager001
 * @tc.desc: Test DataFlowStatisticsTest GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager001, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetCellularRxBytes();
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager002
 * @tc.desc: Test DataFlowStatisticsTest GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager002, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetCellularTxBytes();
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager003
 * @tc.desc: Test DataFlowStatisticsTest GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager003, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetAllRxBytes();
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager004
 * @tc.desc: Test DataFlowStatisticsTest GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager004, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetAllTxBytes();
    ASSERT_GE(ret, 0);
}

HWTEST_F(DataFlowStatisticsTest, NetStatsManager005, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetUidTxBytes(TEST_UID);
    ASSERT_GE(ret, -1);
    ret = flow->GetUidRxBytes(TEST_UID);
    ASSERT_GE(ret, -1);
}

/**
 * @tc.name: NetStatsManager007
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager007, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceRxBytes(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager008
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager008, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceTxBytes(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager009
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceRxPackets.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager009, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceRxPackets(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager010
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceTxPackets.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager010, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceTxPackets(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager011
 * @tc.desc: Test DataFlowStatisticsTest RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager011, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    sptr<NetStatsCallbackTest> callback = GetINetStatsCallbackSample();
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    result = DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
