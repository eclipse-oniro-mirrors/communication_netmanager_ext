/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "ethernet_service_stub.h"
#include "iethernet_service.h"
#include "net_manager_constants.h"
#include "configuration_parcel_ipc.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *TEST_STRING = "test";

class MockEthernetServiceStubTest : public EthernetServiceStub {
public:
    MockEthernetServiceStubTest() = default;
    ~MockEthernetServiceStubTest() override {}
    int32_t GetMacAddress(std::vector<MacAddressInfo> &mai) override
    {
        return 0;
    }

    int32_t SetIfaceConfig(const std::string &iface, const sptr<InterfaceConfiguration> &ic) override
    {
        return 0;
    }

    int32_t GetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic) override
    {
        return 0;
    }

    int32_t IsIfaceActive(const std::string &iface, int32_t &activeStatus) override
    {
        return 0;
    }

    int32_t GetAllActiveIfaces(std::vector<std::string> &activeIfaces) override
    {
        return 0;
    }

    int32_t ResetFactory() override
    {
        return 0;
    }

    int32_t RegisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback) override
    {
        return 0;
    }

    int32_t UnregisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback) override
    {
        return 0;
    }

    int32_t SetInterfaceUp(const std::string &iface) override
    {
        return 0;
    }

    int32_t SetInterfaceDown(const std::string &iface) override
    {
        return 0;
    }

    int32_t GetInterfaceConfig(const std::string &iface, ConfigurationParcelIpc &cfgIpc) override
    {
        return 0;
    }

    int32_t SetInterfaceConfig(const std::string &iface, const ConfigurationParcelIpc &cfgIpc) override
    {
        return 0;
    }
};
} // namespace

using namespace testing::ext;
class EthernetServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<EthernetServiceStub> instance_ = std::make_shared<MockEthernetServiceStubTest>();
};

void EthernetServiceStubTest::SetUpTestCase() {}

void EthernetServiceStubTest::TearDownTestCase() {}

void EthernetServiceStubTest::SetUp() {}

void EthernetServiceStubTest::TearDown() {}

/**
 * @tc.name: OnGetMacAddressTest001
 * @tc.desc: Test EthernetServiceStub OnGetMacAddress.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnGetMacAddressTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_MAC_ADDRESS), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnSetIfaceConfigTest001
 * @tc.desc: Test EthernetServiceStub OnSetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnSetIfaceConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_SET_IFACE_CONFIG),
                                             data, reply, option);
    EXPECT_EQ(ret, 5);
}

/**
 * @tc.name: OnGetIfaceConfigTest001
 * @tc.desc: Test EthernetServiceStub OnGetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnGetIfaceConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_IFACE_CONFIG),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnIsIfaceActiveTest001
 * @tc.desc: Test EthernetServiceStub OnIsIfaceActive.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnIsIfaceActiveTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_IS_IFACE_ACTIVE),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnGetAllActiveIfacesTest001
 * @tc.desc: Test EthernetServiceStub OnGetAllActiveIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnGetAllActiveIfacesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_GET_ALL_ACTIVE_IFACES), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnResetFactoryTest001
 * @tc.desc: Test EthernetServiceStub OnResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnResetFactoryTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_RESET_FACTORY),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnRegisterIfacesStateChangedTest001
 * @tc.desc: Test EthernetServiceStub OnRegisterIfacesStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnRegisterIfacesStateChangedTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_REGISTER_IFACES_STATE_CHANGED), data, reply, option);
    EXPECT_EQ(ret, 5);
}

/**
 * @tc.name: OnUnregisterIfacesStateChangedTest001
 * @tc.desc: Test EthernetServiceStub OnUnregisterIfacesStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnUnregisterIfacesStateChangedTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_UNREGISTER_IFACES_STATE_CHANGED), data, reply, option);
    EXPECT_EQ(ret, 5);
}

/**
 * @tc.name: OnSetInterfaceUpTest001
 * @tc.desc: Test EthernetServiceStub OnSetInterfaceUp.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnSetInterfaceUpTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_SET_INTERFACE_UP),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnSetInterfaceDownTest001
 * @tc.desc: Test EthernetServiceStub OnSetInterfaceDown.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnSetInterfaceDownTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_SET_INTERFACE_DOWN), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnGetInterfaceConfigTest001
 * @tc.desc: Test EthernetServiceStub OnGetInterfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnGetInterfaceConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_GET_INTERFACE_CONFIG), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: OnSetInterfaceConfigTest001
 * @tc.desc: Test EthernetServiceStub OnSetInterfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnSetInterfaceConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_SET_INTERFACE_CONFIG), data, reply, option);
    EXPECT_EQ(ret, 5);
}
} // namespace NetManagerStandard
} // namespace OHOS
