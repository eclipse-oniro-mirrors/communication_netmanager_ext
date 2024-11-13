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

#include <memory>
#include <random>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#endif
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"
#include "net_stats_data_handler.h"
#include "net_stats_database_defines.h"
#include "net_stats_database_helper.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;

using namespace testing::ext;
namespace {
constexpr uint32_t UID_MAX_TEST = 200;
constexpr uint32_t MAX_TEST_DATA = 100;
constexpr uint32_t UID = 2000222;
const std::vector<std::string> MOCK_IFACE = {"wlan0", "eth0", "eth1", "usb0", "wlan1", "usb1"};
std::random_device g_rd;
std::mt19937 g_regn(g_rd());
uint32_t GetUint32()
{
    return static_cast<uint32_t>(g_regn()) % UID_MAX_TEST;
}

uint64_t GetUint64()
{
    return static_cast<uint64_t>(g_regn());
}

std::string GetMockIface()
{
    return MOCK_IFACE.at(g_regn() % MOCK_IFACE.size());
}
std::vector<NetStatsInfo> g_statsData;

NetStatsInfo CreateMockStatsInfo()
{
    NetStatsInfo info;
    info.uid_ = GetUint32();
    info.ident_ = std::to_string(GetUint32());
    info.date_ = GetUint64();
    info.iface_ = GetMockIface();
    info.rxBytes_ = GetUint64();
    info.rxPackets_ = GetUint64();
    info.txBytes_ = GetUint64();
    info.txPackets_ = GetUint64();
    return info;
}

void CreateMockStatsData()
{
    g_statsData.clear();
    for (uint32_t i = 0; i < MAX_TEST_DATA; i++) {
        NetStatsInfo info;
        info.uid_ = GetUint32();
        info.ident_ = std::to_string(GetUint32());
        info.date_ = GetUint64();
        info.iface_ = GetMockIface();
        info.rxBytes_ = GetUint64();
        info.rxPackets_ = GetUint64();
        info.txBytes_ = GetUint64();
        info.txPackets_ = GetUint64();
        g_statsData.push_back(info);
    }
}

void ClearMockStatsData()
{
    g_statsData.clear();
}
} // namespace

class NetStatsDataHandlerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetStatsDataHandlerTest::SetUpTestCase()
{
    CreateMockStatsData();
}

void NetStatsDataHandlerTest::TearDownTestCase()
{
    ClearMockStatsData();
}

void NetStatsDataHandlerTest::SetUp() {}

void NetStatsDataHandlerTest::TearDown() {}

HWTEST_F(NetStatsDataHandlerTest, WriteStatsDataTest001, TestSize.Level1)
{
    NetStatsDataHandler handler;
    CreateMockStatsData();
    int32_t ret = handler.WriteStatsData(g_statsData, UID_TABLE);
    ClearMockStatsData();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, WriteStatsDataTest002, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> mockEmptyStatsData;
    int32_t ret = handler.WriteStatsData(mockEmptyStatsData, UID_TABLE);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetStatsDataHandlerTest, WriteStatsDataTest003, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> mockStatsData;
    mockStatsData.push_back(CreateMockStatsInfo());
    std::string mockEmptyIfaceName;
    int32_t ret = handler.WriteStatsData(mockStatsData, mockEmptyIfaceName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetStatsDataHandlerTest, WriteStatsDataTest004, TestSize.Level1)
{
    NetStatsDataHandler handler;
    CreateMockStatsData();
    int32_t ret = handler.WriteStatsData(g_statsData, IFACE_TABLE);
    ClearMockStatsData();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, WriteStatsDataTest005, TestSize.Level1)
{
    NetStatsDataHandler handler;
    CreateMockStatsData();
    int32_t ret = handler.WriteStatsData(g_statsData, UID_SIM_TABLE);
    ClearMockStatsData();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, ReadStatsDataTest001, TestSize.Level1)
{
    NETMGR_LOG_E("NetStatsDataHandlerTest ReadStatsDataTest001 enter");
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> infos;
    int32_t ret = handler.ReadStatsData(infos, 0, LONG_MAX);
    std::cout << "Data size: " << infos.size() << std::endl;
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, ReadStatsDataTest002, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> infos;
    std::string iface;
    int32_t ret = handler.ReadStatsData(infos, iface, 0, LONG_MAX);
    std::cout << "Data size: " << infos.size() << std::endl;
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetStatsDataHandlerTest, ReadStatsDataTest003, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> infos;
    std::string iface = "testIface";
    int32_t ret = handler.ReadStatsData(infos, iface, 0, LONG_MAX);
    std::cout << "Data size: " << infos.size() << std::endl;
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, ReadStatsDataTest004, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> infos;
    uint32_t testUid = 122;
    std::string emptyIface = "";
    int32_t ret = handler.ReadStatsData(infos, emptyIface, testUid, 0, LONG_MAX);
    std::cout << "Data size: " << infos.size() << std::endl;
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetStatsDataHandlerTest, ReadStatsDataTest005, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> infos;
    uint32_t testUid = 122;
    std::string iface = "testIface";
    int32_t ret = handler.ReadStatsData(infos, iface, 0, testUid, LONG_MAX);
    std::cout << "Data size: " << infos.size() << std::endl;
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, ReadStatsDataTest006, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> infos;
    std::string ident = "2";
    int32_t ret = handler.ReadStatsDataByIdent(infos, ident, 0, LONG_MAX);
    std::cout << "Data size: " << infos.size() << std::endl;
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, ReadStatsDataTest007, TestSize.Level1)
{
    NetStatsDataHandler handler;
    std::vector<NetStatsInfo> infos;
    uint32_t uid = UID;
    std::string ident = "2";
    int32_t ret = handler.ReadStatsData(infos, uid, ident, 0, LONG_MAX);
    std::cout << "Data size: " << infos.size() << std::endl;
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsDataHandlerTest, NetStatsDataHandlerBranchTest001, TestSize.Level1)
{
    NetStatsDataHandler handler;
    uint64_t uid = 100;
    int32_t ret = handler.DeleteByUid(uid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string tableName = "";
    ret = handler.DeleteByDate(tableName, 0, 0);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    ret = handler.ClearData();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
