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
#include "net_eap_callback_stub.h"
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

    int32_t RegCustomEapHandler(int netType, const std::string &regCmd,
        const sptr<INetEapPostbackCallback> &callback) override
    {
        return 0;
    }
 
    int32_t ReplyCustomEapData(int result, const sptr<EapData> &eapData) override
    {
        return 0;
    }
 
    int32_t RegisterCustomEapCallback(int netType, const sptr<INetRegisterEapCallback> &callback) override
    {
        return 0;
    }
 
    int32_t UnRegisterCustomEapCallback(int netType, const sptr<INetRegisterEapCallback> &callback) override
    {
        return 0;
    }
 
    int32_t NotifyWpaEapInterceptInfo(int netType, const sptr<EapData> &eapData) override
    {
        return 0;
    }
 
    int32_t GetDeviceInformation(std::vector<EthernetDeviceInfo> &deviceInfoList) override
    {
        return 0;
    }

    int32_t StartEthEap(int32_t netId, const EthEapProfile& profile) override
    {
        return 0;
    }
 
    int32_t LogOffEthEap(int32_t netId) override
    {
        return 0;
    }
};
} // namespace

class NetRegisterEapCallbackTest : public NetRegisterEapCallbackStub {
public:
    int32_t OnRegisterCustomEapCallback(const std::string &regCmd) override
    {
        return 0;
    }
    int32_t OnReplyCustomEapDataEvent(int result, const sptr<EapData> &eapData) override
    {
        return 0;
    }
};
 
class NetEapPostBackCallbackTest : public NetEapPostbackCallbackStub {
public:
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override
    {
        return 0;
    }
};
 
static inline sptr<NetRegisterEapCallbackTest> g_registerEapCallback = new (std::nothrow) NetRegisterEapCallbackTest();
static inline sptr<NetEapPostBackCallbackTest> g_eapPostbackCallback = new (std::nothrow) NetEapPostBackCallbackTest();
 
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
    EXPECT_LE(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
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
    EXPECT_LE(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
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
    EXPECT_LE(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
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
    EXPECT_LE(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

/**
 * @tc.name: RegCustomEapHandlerTest001
 * @tc.desc: Test EthernetServiceStub RegCustomEapHandler.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, RegCustomEapHandlerTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    int netType = static_cast<int>(NetType::WLAN0);
    if (!data.WriteInt32(netType)) {
        return;
    }
    if (data.WriteRemoteObject(g_eapPostbackCallback->AsObject())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_REG_CUSTOM_EAP_HANDLER), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
 
/**
 * @tc.name: ReplyCustomEapDataTest001
 * @tc.desc: Test EthernetServiceStub ReplyCustomEapData.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, ReplyCustomEapDataTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    int result = 1;
    if (!data.WriteInt32(result)) {
        return;
    }
    sptr<EapData> eapData = new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 25;
    eapData->msgId = 55;
    eapData->bufferLen = 3;
    std::vector<uint8_t> tmp = {0x11, 0x12, 0x13};
    eapData->eapBuffer = tmp;
    if (!eapData->Marshalling(data)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_REPLY_CUSTOM_EAP_DATA), data, reply, option);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == 5);
}
 
/**
 * @tc.name: RegisterCustomEapCallbackTest001
 * @tc.desc: Test EthernetServiceStub RegisterCustomEapCallback.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, RegisterCustomEapCallbackTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    int netType = 1;
    if (!data.WriteInt32(netType)) {
        return;
    }
    if (data.WriteRemoteObject(g_registerEapCallback->AsObject())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_REGISTER_CUSTOM_EAP_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
 
/**
 * @tc.name: UnRegisterCustomEapCallbackTest001
 * @tc.desc: Test EthernetServiceStub UnRegisterCustomEapCallback.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, UnRegisterCustomEapCallbackTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    int netType = 1;
    if (!data.WriteInt32(netType)) {
        return;
    }
    if (data.WriteRemoteObject(g_registerEapCallback->AsObject())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_UN_REGISTER_CUSTOM_EAP_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
 
/**
 * @tc.name: NotifyWpaEapInterceptInfoTest001
 * @tc.desc: Test EthernetServiceStub NotifyWpaEapInterceptInfo.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, NotifyWpaEapInterceptInfoTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor())) {
        return;
    }
    int netType = 1;
    if (!data.WriteInt32(netType)) {
        return;
    }
    sptr<EapData> eapData = new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 25;
    eapData->msgId = 55;
    eapData->bufferLen = 3;
    std::vector<uint8_t> tmp = {0x11, 0x12, 0x13};
    eapData->eapBuffer = tmp;
    if (!eapData->Marshalling(data)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(
        IEthernetServiceIpcCode::COMMAND_NOTIFY_WPA_EAP_INTERCEPT_INFO), data, reply, option);
    EXPECT_TRUE(ret == NETMANAGER_EXT_SUCCESS || ret == 5);
}
 
/**
 * @tc.name: OnGetDeviceInformationTest001
 * @tc.desc: Test EthernetServiceStub OnGetDeviceInformation.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetServiceStubTest, OnGetDeviceInformationTest001, TestSize.Level1)
{
    MessageParcel data;
    data.WriteInterfaceToken(EthernetServiceStub::GetDescriptor());
    data.WriteString(TEST_STRING);
    data.WriteString(TEST_STRING);
    data.WriteInt32(0);
    data.WriteString(TEST_STRING);
    data.WriteString(TEST_STRING);
    data.WriteString(TEST_STRING);
    data.WriteString(TEST_STRING);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(IEthernetServiceIpcCode::COMMAND_GET_DEVICE_INFORMATION), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
