/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <gtest/gtest.h>
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "nrunsolicitedmsgparser.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_client.h"
#include "networkslice_stub.h"
#include "net_manager_constants.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
namespace {
constexpr const char *TEST_STRING = "test";
 
class MockNetworkSliceStubTest : public NetworkSliceStub {
public:
    MockNetworkSliceStubTest() = default;
    ~MockNetworkSliceStubTest() override {}
    int32_t SetNetworkSliceUePolicy(std::vector<uint8_t> buffer) override
    {
        return 0;
    }
    int32_t NetworkSliceInitUePolicy() override
    {
        return 0;
    }
    int32_t NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer) override
    {
        return 0;
    }
    int32_t NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer) override
    {
        return 0;
    }
    int32_t GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode) override
    {
        return 0;
    }
    int32_t GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas) override
    {
        return 0;
    }
    int32_t SetSaState(bool isSaState) override
    {
        return 0;
    }
};
} // namespace
 
using namespace testing::ext;
 
class NetworkSliceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetworkSliceStub> instance_ = std::make_shared<MockNetworkSliceStubTest>();
    static int32_t SendRemoteRequest(MessageParcel &data, NetworkSliceInterfaceCode code);
};
 
void NetworkSliceStubTest::SetUpTestCase() {}
 
void NetworkSliceStubTest::TearDownTestCase() {}
 
void NetworkSliceStubTest::SetUp() {}
 
void NetworkSliceStubTest::TearDown() {}
 
int32_t NetworkSliceStubTest::SendRemoteRequest(MessageParcel &data, NetworkSliceInterfaceCode code)
{
    MessageParcel reply;
    MessageOption option;
    return instance_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

HWTEST_F(NetworkSliceStubTest, OnRemoteRequest002, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceStubTest:OnRemoteRequest002");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(NetworkSliceInterfaceCode::SET_NETWORKSLICE_UEPOLICY), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH);
}
 
HWTEST_F(NetworkSliceStubTest, OnSetNetworkSlicePolicy001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnSetNetworkSlicePolicy001");
    MessageParcel data;
    int32_t buffersize = 1;
    uint8_t buffer = 0;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    data.WriteInt32(buffersize);
    data.WriteUint8(buffer);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::SET_NETWORKSLICE_UEPOLICY);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkSliceStubTest, OnSetNetworkSlicePolicy002, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnSetNetworkSlicePolicy002");
    MessageParcel data;
    int32_t buffersize = 0;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    data.WriteInt32(buffersize);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::SET_NETWORKSLICE_UEPOLICY);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceStubTest, OnNetworkSliceAllowedNssaiRpt001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkSliceAllowedNssaiRpt001");
    MessageParcel data;
    int32_t buffersize = 1;
    uint8_t buffer = 0;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    data.WriteInt32(buffersize);
    data.WriteUint8(buffer);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_ALLOWEDNSSAI_RPT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkSliceStubTest, OnNetworkSliceAllowedNssaiRpt002, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkSliceAllowedNssaiRpt002");
    MessageParcel data;
    int32_t buffersize = 0;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_ALLOWEDNSSAI_RPT);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceStubTest, OnNetworkSliceEhplmnRpt001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkSliceEhplmnRpt001");
    MessageParcel data;
    int32_t buffersize = 1;
    uint8_t buffer = 0;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    data.WriteInt32(buffersize);
    data.WriteUint8(buffer);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_EHPLMN_RPT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkSliceStubTest, OnNetworkSliceEhplmnRpt002, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkSliceEhplmnRpt002");
    MessageParcel data;
    int32_t buffersize = 0;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_EHPLMN_RPT);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetworkSliceStubTest, OnNetworkSliceInitUePolicy001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnNetworkSliceInitUePolicy001");
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_INIT_UEPOLICY);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceStubTest, OnGetRouteSelectionDescriptorByDNN001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnGetRouteSelectionDescriptorByDNN001");
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_GETRSDBYDNN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceStubTest, OnGetRSDByNetCap001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnGetRSDByNetCap001");
    MessageParcel data;
    int32_t netcap = 19;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_GETRSDBYNETCAP);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceStubTest, OnSetSaState001, TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnSetSaState001");
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetworkSliceStub::GetDescriptor()), false);
    int32_t ret = SendRemoteRequest(data, NetworkSliceInterfaceCode::NETWORKSLICE_SETSASTATE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
}
}
