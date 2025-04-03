/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "networkslice_service.h"
#include "networkslicemsgcenter.h"
#include <sys/time.h>
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"
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
#include "networkslice_client.h"
#include "networkslice_stub.h"
#include "net_manager_constants.h"
#include "networkslice_kernel_proxy.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
using namespace testing::ext;
 
class NetworkSliceServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    sptr<INetworkSliceService> GetIfaceConfig();
    void SetUp();
    void TearDown();
    static inline auto instance_ = DelayedSingleton<NetworkSliceService>::GetInstance();
};
 
void NetworkSliceServiceTest::SetUpTestCase() {}
 
void NetworkSliceServiceTest::TearDownTestCase() {}
 
void NetworkSliceServiceTest::SetUp() {}
 
void NetworkSliceServiceTest::TearDown() {}

HWTEST_F(NetworkSliceServiceTest, OnStart001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnStart001");
    instance_->OnStart();
    EXPECT_EQ(instance_->state_, NetworkSliceService::STATE_STOPPED);
}
 
HWTEST_F(NetworkSliceServiceTest, OnStart002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnStart002");
    instance_-> state_ = NetworkSliceService::STATE_RUNNING;
    instance_->OnStart();
    EXPECT_EQ(instance_->state_, NetworkSliceService::STATE_RUNNING);
}
 
HWTEST_F(NetworkSliceServiceTest, OnStop001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("OnStop001");
    NetworkSliceService service;
    service.state_ = NetworkSliceService::STATE_STOPPED;
    service.OnStop();
    EXPECT_FALSE(service.isRegistered_);
}

HWTEST_F(NetworkSliceServiceTest, Init002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("Init002");
    bool result = NetworkSliceService::GetInstance().Init();
    EXPECT_FALSE(result);
    EXPECT_FALSE(NetworkSliceService::GetInstance().isRegistered_);
}

HWTEST_F(NetworkSliceServiceTest, Dump001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("Dump001");
    EXPECT_EQ(NetworkSliceService::GetInstance().Dump(-1, {}), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, SetNetworkSliceUePolicy001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SetNetworkSliceUePolicy001");
    std::vector<uint8_t> buffer = {0x01, 0x01, 0x01, 0x05, 0xE6, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    EXPECT_EQ(NetworkSliceService::GetInstance().SetNetworkSliceUePolicy(buffer), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, NetworkSliceAllowedNssaiRpt001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceAllowedNssaiRpt001");
    std::vector<uint8_t> buffer = {0x41, 0x2E, 0x42, 0x3A, 0x43, 0x2E, 0x44};
    EXPECT_EQ(NetworkSliceService::GetInstance().NetworkSliceAllowedNssaiRpt(buffer), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, NetworkSliceEhplmnRpt001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceEhplmnRpt001");
    std::vector<uint8_t> buffer = {0x41, 0x2C, 0x42};
    EXPECT_EQ(NetworkSliceService::GetInstance().NetworkSliceEhplmnRpt(buffer), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, NetworkSliceInitUePolicy001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceInitUePolicy001");
    EXPECT_EQ(NetworkSliceService::GetInstance().NetworkSliceInitUePolicy(), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, NetworkSliceGetRSDByAppDescriptor001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("NetworkSliceGetRSDByAppDescriptor001");
    std::shared_ptr<GetSlicePara> getSlicePara = std::make_shared<GetSlicePara>();
    EXPECT_EQ(NetworkSliceService::GetInstance().NetworkSliceGetRSDByAppDescriptor(getSlicePara),
        NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, RecvKernelData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RecvKernelData001");
    void* rcvMsg = reinterpret_cast<void*>(new KernelMsgNS());
    int32_t dataLen = sizeof(KernelMsgNS) + 10;
    int32_t result = NetworkSliceService::GetInstance().RecvKernelData(rcvMsg, dataLen);
 
    // 3. Verify the result
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, RecvKernelData002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RecvKernelData002");
    void* rcvMsg = nullptr;
    int32_t dataLen = sizeof(KernelMsgNS);
    int32_t result = NetworkSliceService::GetInstance().RecvKernelData(rcvMsg, dataLen);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}
 
HWTEST_F(NetworkSliceServiceTest, RecvKernelData004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("RecvKernelData004");
    void* rcvMsg = reinterpret_cast<void*>(new KernelMsgNS());
    int32_t dataLen = 1;
    int32_t buflen = 0;
    int32_t result = NetworkSliceService::GetInstance().RecvKernelData(rcvMsg, dataLen);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}
 
HWTEST_F(NetworkSliceServiceTest, GetBundleNameForUid001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetBundleNameForUid001");
    int32_t uid = 12345;
    std::string bundleName;
    int result = NetworkSliceService::GetInstance().GetBundleNameForUid(uid, bundleName);
    EXPECT_NE(result, 0);
    EXPECT_EQ(bundleName, "");
}
 
HWTEST_F(NetworkSliceServiceTest, BindToNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("BindToNetwork001");
    std::map<std::string, std::string> buffer;
    buffer["key"] = "value";
    int32_t result = NetworkSliceService::GetInstance().BindToNetwork(buffer);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(NetworkSliceServiceTest, DelBindToNetwork001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DelBindToNetwork001");
    std::map<std::string, std::string> buffer;
    buffer["key"] = "value";
    int32_t result = NetworkSliceService::GetInstance().DelBindToNetwork(buffer);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkSliceServiceTest, UpdateNetworkSliceApn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UpdateNetworkSliceApn001");
    EXPECT_EQ(false, NetworkSliceService::GetInstance().UpdateNetworkSliceApn());
}
 
}
}
