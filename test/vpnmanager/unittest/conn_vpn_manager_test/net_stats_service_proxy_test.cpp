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

#include <vector>

#include <gtest/gtest.h>

#include "i_net_stats_service.h"
#include "iremote_proxy.h"
#include "net_manager_center.h"
#include "net_stats_callback.h"
#include "net_stats_callback_test.h"
#include "net_stats_constants.h"
#include "net_stats_service.h"
#include "net_stats_service_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
constexpr const char *ETH_IFACE_NAME = "lo";
constexpr int64_t TEST_UID = 1010;
constexpr uint64_t STATS_CODE = 100;
static constexpr uint64_t TEST_COOKIE = 1;
class MockNetIRemoteObject : public IRemoteObject {
public:
    MockNetIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}
    ~MockNetIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        if (!reply.WriteInt32(NETSYS_SUCCESS)) {
            return NETMANAGER_ERROR;
        }
        if (code >= static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_RXBYTES) &&
            code <= static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_TXBYTES)) {
            if (!reply.WriteInt64(STATS_CODE)) {
                return NETMANAGER_ERROR;
            }
        } else if (code == static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_STATS_DETAIL) ||
                   code == static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_STATS_DETAIL) ||
                   code == static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_SIM_STATS_INFO) ||
                   code == static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_TRAFFIC_STATS_BY_NETWORK) ||
                   code == static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_TRAFFIC_STATS_BY_UID_NETWORK)) {
            if (eCode == NETMANAGER_ERR_READ_REPLY_FAIL) {
                return NETSYS_SUCCESS;
            }

            if (!reply.WriteUint32(TEST_UID)) {
                return NETMANAGER_ERROR;
            }
            if (!reply.WriteString("wlan0")) {
                return NETMANAGER_ERROR;
            }
            if (!reply.WriteString("ident0")) {
                return NETMANAGER_ERROR;
            }
            if (!reply.WriteUint64(TEST_UID)) {
                return NETMANAGER_ERROR;
            }
            if (!reply.WriteUint64(TEST_UID)) {
                return NETMANAGER_ERROR;
            }
            if (!reply.WriteUint64(TEST_UID)) {
                return NETMANAGER_ERROR;
            }
            if (!reply.WriteUint64(TEST_UID)) {
                return NETMANAGER_ERROR;
            }
            if (!reply.WriteUint64(TEST_UID)) {
                return NETMANAGER_ERROR;
            }
        }
        return eCode;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }

    void SetErrorCode(int errorCode)
    {
        eCode = errorCode;
    }

private:
    int eCode = 0;
};

} // namespace

using namespace testing::ext;
class NetStatsServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline sptr<MockNetIRemoteObject> remoteObj_ = nullptr;
    static inline sptr<INetStatsCallback> callback_ = nullptr;
};

void NetStatsServiceProxyTest::SetUpTestCase()
{
    remoteObj_ = new (std::nothrow) MockNetIRemoteObject();
    callback_ = new (std::nothrow) NetStatsCallbackTest();
}

void NetStatsServiceProxyTest::TearDownTestCase()
{
    remoteObj_ = nullptr;
    callback_ = nullptr;
}

void NetStatsServiceProxyTest::SetUp() {}

void NetStatsServiceProxyTest::TearDown() {}

/**
 * @tc.name: RegisterNetStatsCallbackTest001
 * @tc.desc: Test NetStatsServiceProxy RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, RegisterNetStatsCallbackTest001, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.RegisterNetStatsCallback(nullptr), NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: RegisterNetStatsCallbackTest002
 * @tc.desc: Test NetStatsServiceProxy RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, RegisterNetStatsCallbackTest002, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.RegisterNetStatsCallback(callback_), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: RegisterNetStatsCallbackTest003
 * @tc.desc: Test NetStatsServiceProxy RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, RegisterNetStatsCallbackTest003, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.RegisterNetStatsCallback(callback_), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: RegisterNetStatsCallbackTest004
 * @tc.desc: Test NetStatsServiceProxy RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, RegisterNetStatsCallbackTest004, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.RegisterNetStatsCallback(callback_), NETSYS_SUCCESS);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest001
 * @tc.desc: Test NetStatsServiceProxy UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UnregisterNetStatsCallbackTest001, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.UnregisterNetStatsCallback(nullptr), NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest002
 * @tc.desc: Test NetStatsServiceProxy UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UnregisterNetStatsCallbackTest002, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.UnregisterNetStatsCallback(callback_), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest003
 * @tc.desc: Test NetStatsServiceProxy UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UnregisterNetStatsCallbackTest003, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.UnregisterNetStatsCallback(callback_), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest004
 * @tc.desc: Test NetStatsServiceProxy UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UnregisterNetStatsCallbackTest004, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.UnregisterNetStatsCallback(callback_), NETSYS_SUCCESS);
}

/**
 * @tc.name: GetIfaceRxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetIfaceRxBytes(stats, ETH_IFACE_NAME), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetIfaceRxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceRxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetIfaceRxBytes(stats, ETH_IFACE_NAME), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetIfaceRxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceRxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetIfaceRxBytes(stats, ETH_IFACE_NAME), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetIfaceTxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetIfaceTxBytes(stats, ETH_IFACE_NAME), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetIfaceTxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceTxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetIfaceTxBytes(stats, ETH_IFACE_NAME), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetIfaceTxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceTxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetIfaceTxBytes(stats, ETH_IFACE_NAME), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetCellularRxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetCellularRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetCellularRxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetCellularRxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetCellularRxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCellularRxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetCellularRxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetCellularRxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCellularRxBytes(stats), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetCellularTxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetCellularTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetCellularTxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetCellularTxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetCellularTxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCellularTxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetCellularTxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetCellularTxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCellularTxBytes(stats), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetAllRxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetAllRxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetAllRxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllRxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetAllRxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetAllRxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllRxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetAllRxBytes(stats), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetAllTxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetAllTxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetAllTxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllTxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetAllTxBytes(stats), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetAllTxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllTxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetAllTxBytes(stats), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetUidRxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetUidRxBytes(stats, TEST_UID), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetUidRxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidRxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetUidRxBytes(stats, TEST_UID), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetUidRxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidRxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetUidRxBytes(stats, TEST_UID), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetUidTxBytesTest001
 * @tc.desc: Test NetStatsServiceProxy GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetUidTxBytes(stats, TEST_UID), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetUidTxBytesTest002
 * @tc.desc: Test NetStatsServiceProxy GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidTxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetUidTxBytes(stats, TEST_UID), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetUidTxBytesTest003
 * @tc.desc: Test NetStatsServiceProxy GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidTxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetUidTxBytes(stats, TEST_UID), NETSYS_SUCCESS);
    EXPECT_EQ(stats, STATS_CODE);
}

/**
 * @tc.name: GetIfaceStatsDetailTest001
 * @tc.desc: Test NetStatsServiceProxy GetIfaceStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceStatsDetailTest001, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetIfaceStatsDetail(iface, 0, UINT32_MAX, info), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetIfaceStatsDetailTest002
 * @tc.desc: Test NetStatsServiceProxy GetIfaceStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceStatsDetailTest002, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetIfaceStatsDetail(iface, 0, UINT32_MAX, info), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetIfaceStatsDetailTest003
 * @tc.desc: Test NetStatsServiceProxy GetIfaceStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceStatsDetailTest003, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    remoteObj_->SetErrorCode(NETMANAGER_ERR_READ_REPLY_FAIL);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetIfaceStatsDetail(iface, 0, UINT32_MAX, info), NETMANAGER_ERR_READ_REPLY_FAIL);
}

/**
 * @tc.name: GetIfaceStatsDetailTest004
 * @tc.desc: Test NetStatsServiceProxy GetIfaceStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetIfaceStatsDetailTest004, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetIfaceStatsDetail(iface, 0, UINT32_MAX, info), NETSYS_SUCCESS);
    EXPECT_EQ(info.uid_, TEST_UID);
}

/**
 * @tc.name: GetUidStatsDetailTest001
 * @tc.desc: Test NetStatsServiceProxy GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidStatsDetailTest001, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    uint32_t uid = 1234;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetUidStatsDetail(iface, uid, 0, UINT32_MAX, info), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetUidStatsDetailTest002
 * @tc.desc: Test NetStatsServiceProxy GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidStatsDetailTest002, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    uint32_t uid = 1234;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetUidStatsDetail(iface, uid, 0, UINT32_MAX, info), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetUidStatsDetailTest003
 * @tc.desc: Test NetStatsServiceProxy GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidStatsDetailTest003, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    uint32_t uid = 1234;
    remoteObj_->SetErrorCode(NETMANAGER_ERR_READ_REPLY_FAIL);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetUidStatsDetail(iface, uid, 0, UINT32_MAX, info), NETMANAGER_ERR_READ_REPLY_FAIL);
}

/**
 * @tc.name: GetUidStatsDetailTest004
 * @tc.desc: Test NetStatsServiceProxy GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetUidStatsDetailTest004, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    uint32_t uid = 1234;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetUidStatsDetail(iface, uid, 0, UINT32_MAX, info), NETSYS_SUCCESS);
    EXPECT_EQ(info.uid_, TEST_UID);
}

/**
 * @tc.name: GetAllSimStatsInfoTest001
 * @tc.desc: Test NetStatsServiceProxy GetAllSimStatsInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllSimStatsInfoTest001, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    std::vector<NetStatsInfo> infos;
    EXPECT_EQ(instance_.GetAllSimStatsInfo(infos), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetAllSimStatsInfoTest002
 * @tc.desc: Test NetStatsServiceProxy GetAllSimStatsInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllSimStatsInfoTest002, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    std::vector<NetStatsInfo> infos;
    EXPECT_EQ(instance_.GetAllSimStatsInfo(infos), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetAllSimStatsInfoTest003
 * @tc.desc: Test NetStatsServiceProxy GetAllSimStatsInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllSimStatsInfoTest003, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERR_READ_REPLY_FAIL);
    NetStatsServiceProxy instance_(remoteObj_);
    std::vector<NetStatsInfo> infos;
    EXPECT_EQ(instance_.GetAllSimStatsInfo(infos), NETMANAGER_ERR_READ_REPLY_FAIL);
}

/**
 * @tc.name: GetAllSimStatsInfoTest004
 * @tc.desc: Test NetStatsServiceProxy GetAllSimStatsInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetAllSimStatsInfoTest004, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    std::vector<NetStatsInfo> infos;
    EXPECT_EQ(instance_.GetAllSimStatsInfo(infos), NETSYS_SUCCESS);
}

/**
 * @tc.name: GetTrafficStatsByNetworkTest001
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByNetworkTest001, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByNetwork(infos, network), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetTrafficStatsByNetworkTest002
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByNetworkTest002, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByNetwork(infos, network), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetTrafficStatsByNetworkTest003
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByNetworkTest003, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERR_READ_REPLY_FAIL);
    NetStatsServiceProxy instance_(remoteObj_);
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByNetwork(infos, network), NETMANAGER_ERR_READ_REPLY_FAIL);
}

/**
 * @tc.name: GetTrafficStatsByNetworkTest004
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByNetworkTest004, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByNetwork(infos, network), NETSYS_SUCCESS);
}

/**
 * @tc.name: GetTrafficStatsByUidNetworkTest001
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByUidNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByUidNetworkTest001, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    std::vector<NetStatsInfoSequence> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByUidNetwork(infos, TEST_UID, network), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetTrafficStatsByUidNetworkTest002
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByUidNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByUidNetworkTest002, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    std::vector<NetStatsInfoSequence> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByUidNetwork(infos, TEST_UID, network), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GetTrafficStatsByUidNetworkTest003
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByUidNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByUidNetworkTest003, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERR_READ_REPLY_FAIL);
    NetStatsServiceProxy instance_(remoteObj_);
    std::vector<NetStatsInfoSequence> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByUidNetwork(infos, TEST_UID, network), NETMANAGER_ERR_READ_REPLY_FAIL);
}

/**
 * @tc.name: GetTrafficStatsByUidNetworkTest004
 * @tc.desc: Test NetStatsServiceProxy GetTrafficStatsByUidNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, GetTrafficStatsByUidNetworkTest004, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    std::vector<NetStatsInfoSequence> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    EXPECT_EQ(instance_.GetTrafficStatsByUidNetwork(infos, TEST_UID, network), NETSYS_SUCCESS);
}

/**
 * @tc.name: SetAppStatsTest001
 * @tc.desc: Test NetStatsServiceProxy SetAppStats.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, SetAppStatsTest001, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    PushStatsInfo info;
    EXPECT_EQ(instance_.SetAppStats(info), NETSYS_SUCCESS);
}

/**
 * @tc.name: UpdateIfacesStatsTest001
 * @tc.desc: Test NetStatsServiceProxy UpdateIfacesStats.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UpdateIfacesStatsTest001, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.UpdateIfacesStats(iface, 0, UINT32_MAX, info), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UpdateIfacesStatsTest002
 * @tc.desc: Test NetStatsServiceProxy UpdateIfacesStats.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UpdateIfacesStatsTest002, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.UpdateIfacesStats(iface, 0, UINT32_MAX, info), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UpdateIfacesStatsTest003
 * @tc.desc: Test NetStatsServiceProxy UpdateIfacesStats.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UpdateIfacesStatsTest003, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.UpdateIfacesStats(iface, 0, UINT32_MAX, info), NETSYS_SUCCESS);
}

/**
 * @tc.name: UpdateStatsDataTest001
 * @tc.desc: Test NetStatsServiceProxy UpdateStatsData.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UpdateStatsDataTest001, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.UpdateStatsData(), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UpdateStatsDataTest002
 * @tc.desc: Test NetStatsServiceProxy UpdateStatsData.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UpdateStatsDataTest002, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.UpdateStatsData(), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UpdateStatsDataTest003
 * @tc.desc: Test NetStatsServiceProxy UpdateStatsData.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, UpdateStatsDataTest003, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.UpdateStatsData(), NETSYS_SUCCESS);
}

HWTEST_F(NetStatsServiceProxyTest, GetCookieRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetCookieRxBytes(stats, TEST_COOKIE), NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetStatsServiceProxyTest, GetCookieRxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCookieRxBytes(stats, TEST_COOKIE), NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetStatsServiceProxyTest, GetCookieRxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCookieRxBytes(stats, TEST_COOKIE), NETMANAGER_ERR_READ_REPLY_FAIL);
}

HWTEST_F(NetStatsServiceProxyTest, GetCookieTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.GetCookieTxBytes(stats, TEST_COOKIE), NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetStatsServiceProxyTest, GetCookieTxBytesTest002, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCookieTxBytes(stats, TEST_COOKIE), NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetStatsServiceProxyTest, GetCookieTxBytesTest003, TestSize.Level1)
{
    uint64_t stats = 0;
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.GetCookieTxBytes(stats, TEST_COOKIE), NETMANAGER_ERR_READ_REPLY_FAIL);
}

/**
 * @tc.name: ResetFactoryTest001
 * @tc.desc: Test NetStatsServiceProxy ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, ResetFactoryTest001, TestSize.Level1)
{
    NetStatsServiceProxy instance_(nullptr);
    EXPECT_EQ(instance_.ResetFactory(), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: ResetFactoryTest002
 * @tc.desc: Test NetStatsServiceProxy ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, ResetFactoryTest002, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_ERROR);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.ResetFactory(), NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: ResetFactoryTest003
 * @tc.desc: Test NetStatsServiceProxy ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceProxyTest, ResetFactoryTest003, TestSize.Level1)
{
    remoteObj_->SetErrorCode(NETMANAGER_SUCCESS);
    NetStatsServiceProxy instance_(remoteObj_);
    EXPECT_EQ(instance_.ResetFactory(), NETSYS_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
