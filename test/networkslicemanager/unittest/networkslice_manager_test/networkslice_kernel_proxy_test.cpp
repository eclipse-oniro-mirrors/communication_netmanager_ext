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
#include "networkslice_kernel_proxy.h"
#include <map>
#include <mutex>
#include <set>
#include <vector>
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_service_base.h"
#include "networkslice_client.h"
#include "state_utils.h"
 
namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t NETLINK_BUFFER_MAX_SIZE = 4080;
constexpr int32_t NETWORKSLICE_REG_MSG = 0;
const std::vector<int16_t> MSG_TYPE_VEC = {
    KERNEL_RSP_SLICE_IP_PARA};
} // namespace
class ServiceBaseTest : public NetworkSliceServiceBase {
    DECLARE_DELAYED_SINGLETON(ServiceBaseTest);
public:
    void OnInit() override;
};
 
ServiceBaseTest::ServiceBaseTest()
    : NetworkSliceServiceBase(MODULE_NETWORKSLICE)
{
}
 
ServiceBaseTest::~ServiceBaseTest()
{
}
 
void ServiceBaseTest::OnInit()
{
}
 
class NetworkSliceKernelProxyTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp();
    void TearDown() {};
public:
    static int32_t netlinkSocketTest_;
};
 
int32_t NetworkSliceKernelProxyTest::netlinkSocketTest_ =
    DelayedSingleton<NetworkSliceKernelProxy>::GetInstance()->StartNetlink();
 
void NetworkSliceKernelProxyTest::SetUp()
{
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->moduleIdHandlerMap_.clear();
    kernelProxy->moduleIdMsgTypesMap_.clear();
    kernelProxy->msgTypeModuleIdsMap_.clear();
    kernelProxy->netlinkSocket_ = -1;
}

/**
* @tc.name: RegistHandler001
* @tc.desc: RegistHandler func test
* @tc.type: FUNC
* @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, RegistHandler001, testing::ext::TestSize.Level1)
{
    auto serviceBaseTest = DelayedSingleton<ServiceBaseTest>::GetInstance();
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, serviceBaseTest.get(), MSG_TYPE_VEC);
    EXPECT_EQ(kernelProxy->moduleIdHandlerMap_.size(), 1);
    EXPECT_EQ(kernelProxy->moduleIdMsgTypesMap_.size(), 1);
    EXPECT_EQ(kernelProxy->msgTypeModuleIdsMap_.size(), MSG_TYPE_VEC.size());
}
 
/**
 * @tc.name: RegistHandler002
 * @tc.desc: RegistHandler func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, RegistHandler002, testing::ext::TestSize.Level1)
{
    auto serviceBaseTest = DelayedSingleton<ServiceBaseTest>::GetInstance();
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, serviceBaseTest.get(), MSG_TYPE_VEC);
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, serviceBaseTest.get(), MSG_TYPE_VEC);
    EXPECT_EQ(kernelProxy->moduleIdHandlerMap_.size(), 1);
    EXPECT_EQ(kernelProxy->moduleIdMsgTypesMap_.size(), 1);
    EXPECT_EQ(kernelProxy->msgTypeModuleIdsMap_.size(), MSG_TYPE_VEC.size());
}
 
/**
 * @tc.name: RegistHandler003
 * @tc.desc: RegistHandler func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, RegistHandler003, testing::ext::TestSize.Level1)
{
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, nullptr, MSG_TYPE_VEC);
    EXPECT_EQ(kernelProxy->moduleIdHandlerMap_.size(), 0);
    EXPECT_EQ(kernelProxy->moduleIdMsgTypesMap_.size(), 0);
    EXPECT_EQ(kernelProxy->msgTypeModuleIdsMap_.size(), 0);
}
 
/**
 * @tc.name: RegistHandler004
 * @tc.desc: RegistHandler func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, RegistHandler004, testing::ext::TestSize.Level1)
{
    auto serviceBaseTest = DelayedSingleton<ServiceBaseTest>::GetInstance();
    std::vector<int16_t> emptyMsgTypeVec;
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, serviceBaseTest.get(), emptyMsgTypeVec);
    EXPECT_EQ(kernelProxy->moduleIdHandlerMap_.size(), 0);
    EXPECT_EQ(kernelProxy->moduleIdMsgTypesMap_.size(), 0);
    EXPECT_EQ(kernelProxy->msgTypeModuleIdsMap_.size(), 0);
}
/**
 * @tc.name: UnRegistHandler001
 * @tc.desc: UnRegistHandler func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, UnRegistHandler001, testing::ext::TestSize.Level1)
{
    auto serviceBaseTest = DelayedSingleton<ServiceBaseTest>::GetInstance();
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, serviceBaseTest.get(), MSG_TYPE_VEC);
    kernelProxy->UnRegistHandler(MODULE_NETWORKSLICE);
    EXPECT_EQ(kernelProxy->moduleIdHandlerMap_.size(), 0);
}
 
/**
 * @tc.name: UnRegistHandler002
 * @tc.desc: UnRegistHandler func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, UnRegistHandler002, testing::ext::TestSize.Level1)
{
    auto serviceBaseTest = DelayedSingleton<ServiceBaseTest>::GetInstance();
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, serviceBaseTest.get(), MSG_TYPE_VEC);
    kernelProxy->UnRegistHandler(MODULE_COMMON);
    EXPECT_EQ(kernelProxy->moduleIdHandlerMap_.size(), 1);
}
 
/**
 * @tc.name: SendMsgToKernel001
 * @tc.desc: SendMsgToKernel func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, SendMsgToKernel001, testing::ext::TestSize.Level1)
{
    int32_t type = NETWORKSLICE_REG_MSG;
    nlmsghdr *nlmsg = nullptr;
    size_t nlmsgLen = NLMSG_LENGTH(0);
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->netlinkSocket_ = netlinkSocketTest_;
 
    int32_t ret = kernelProxy->SendMsgToKernel(type, nlmsg, nlmsgLen);
    EXPECT_EQ(ret, -1);
}
 
/**
 * @tc.name: SendMsgToKernel002
 * @tc.desc: SendMsgToKernel func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, SendMsgToKernel002, testing::ext::TestSize.Level1)
{
    int32_t type = NETWORKSLICE_REG_MSG;
    NetlinkInfo nlreq = {};
    size_t nlmsgLen = NLMSG_LENGTH(0);
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->netlinkSocket_ = -1;
    int32_t ret = kernelProxy->SendMsgToKernel(type, reinterpret_cast<nlmsghdr*>(&nlreq), nlmsgLen);
    EXPECT_EQ(ret, -1);
}
 
/**
 * @tc.name: SendMsgToKernel003
 * @tc.desc: SendMsgToKernel func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, SendMsgToKernel003, testing::ext::TestSize.Level1)
{
    int32_t type = NETWORKSLICE_REG_MSG;
    NetlinkInfo nlreq = {};
    size_t nlmsgLen = 0;
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->netlinkSocket_ = netlinkSocketTest_;
 
    int32_t ret = kernelProxy->SendMsgToKernel(type, reinterpret_cast<nlmsghdr*>(&nlreq), nlmsgLen);
    EXPECT_EQ(ret, -1);
}
 
/**
 * @tc.name: SendMsgToKernel004
 * @tc.desc: SendMsgToKernel func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, SendMsgToKernel004, testing::ext::TestSize.Level1)
{
    int32_t type = NETWORKSLICE_REG_MSG;
    NetlinkInfo nlreq = {};
    size_t nlmsgLen = sizeof(nlmsghdr) + 1;
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->netlinkSocket_ = netlinkSocketTest_;
    int32_t ret = kernelProxy->SendMsgToKernel(type, reinterpret_cast<nlmsghdr*>(&nlreq), nlmsgLen);
    EXPECT_TRUE(ret >= -1);
}
 
/**
 * @tc.name: SendDataToKernel001
 * @tc.desc: SendDataToKernel func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, SendDataToKernel001, testing::ext::TestSize.Level1)
{
    KernelMsg msgData;
    msgData.len = 0;
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->netlinkSocket_ = netlinkSocketTest_;
 
    int32_t ret = kernelProxy->SendDataToKernel(msgData);
    EXPECT_EQ(ret, -1);
}
 
/**
 * @tc.name: SendDataToKernel002
 * @tc.desc: SendDataToKernel func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, SendDataToKernel002, testing::ext::TestSize.Level1)
{
    KernelMsg msgData;
    msgData.len = NETLINK_BUFFER_MAX_SIZE + 1;
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->netlinkSocket_ = netlinkSocketTest_;
 
    int32_t ret = kernelProxy->SendDataToKernel(msgData);
    EXPECT_EQ(ret, -1);
}
 
/**
 * @tc.name: SendDataToKernel003
 * @tc.desc: SendDataToKernel func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, SendDataToKernel003, testing::ext::TestSize.Level1)
{
    KernelIpRptEnableMsg kernelIpRptEnableMsg;
    kernelIpRptEnableMsg.type = 9;
    kernelIpRptEnableMsg.len = 12;
    kernelIpRptEnableMsg.isEnable = 1;
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->netlinkSocket_ = netlinkSocketTest_;
    int32_t ret = kernelProxy->SendDataToKernel(reinterpret_cast<KernelMsg&>(kernelIpRptEnableMsg));
    EXPECT_TRUE(ret >= -1);
}
 
/**
 * @tc.name: RecvKernelData001
 * @tc.desc: RecvKernelData func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, RecvKernelData001, testing::ext::TestSize.Level1)
{
    auto serviceBaseTest = DelayedSingleton<ServiceBaseTest>::GetInstance();
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    kernelProxy->RegistHandler(MODULE_NETWORKSLICE, serviceBaseTest.get(), MSG_TYPE_VEC);
    kernelProxy->netlinkSocket_ = netlinkSocketTest_;
 
    kernelProxy->RecvKernelData(kernelProxy.get(), 0);
    EXPECT_EQ(kernelProxy->moduleIdHandlerMap_.size(), 1);
}
 
/**
 * @tc.name: DispatchKernelMsg001
 * @tc.desc: DispatchKernelMsg func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, DispatchKernelMsg001, testing::ext::TestSize.Level1)
{
    KernelMsg kernelMsg;
    int32_t dataLen = static_cast<int32_t>(sizeof(KernelMsg));
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    EXPECT_NO_THROW(kernelProxy->DispatchKernelMsg(&kernelMsg, dataLen));
}
 
/**
 * @tc.name: DispatchKernelMsg002
 * @tc.desc: DispatchKernelMsg func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, DispatchKernelMsg002, testing::ext::TestSize.Level1)
{
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    EXPECT_NO_THROW(kernelProxy->DispatchKernelMsg(nullptr, 0));
}
 
/**
 * @tc.name: IsValidDataLen001
 * @tc.desc: IsValidDataLen func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, IsValidDataLen001, testing::ext::TestSize.Level1)
{
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    int32_t dataLen = 0;
    bool ret = kernelProxy->IsValidDataLen(dataLen);
    EXPECT_EQ(ret, false);
}
 
/**
 * @tc.name: IsValidDataLen002
 * @tc.desc: IsValidDataLen func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, IsValidDataLen002, testing::ext::TestSize.Level1)
{
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    int32_t dataLen = NETLINK_BUFFER_MAX_SIZE + 1;
    bool ret = kernelProxy->IsValidDataLen(dataLen);
    EXPECT_EQ(ret, false);
}
 
/**
 * @tc.name: IsValidDataLen003
 * @tc.desc: IsValidDataLen func test
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(NetworkSliceKernelProxyTest, IsValidDataLen003, testing::ext::TestSize.Level1)
{
    auto kernelProxy = DelayedSingleton<NetworkSliceKernelProxy>::GetInstance();
    int32_t dataLen = static_cast<int32_t>(sizeof(KernelMsg));
    bool ret = kernelProxy->IsValidDataLen(dataLen);
    EXPECT_EQ(ret, true);
}
}
}
