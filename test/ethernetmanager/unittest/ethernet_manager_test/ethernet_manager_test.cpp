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

#include <gtest/gtest.h>

#include <fstream>

#include "ethernet_client.h"
#include "http_proxy.h"
#include "inet_addr.h"
#include "mac_address_info.h"
#include "interface_configuration.h"
#include "interface_state_callback_stub.h"
#include "interface_type.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "parameters.h"
#include "refbase.h"
#include "singleton.h"
#include "static_configuration.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"

#define private public
#define protected public
#include "ethernet_client.h"
#include "ethernet_dhcp_controller.h"
#include "ethernet_management.h"
#include "ethernet_service.h"
#include "ethernet_service_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *DEV_NAME = "eth0";
constexpr const char *DEV_UP = "up";
constexpr const char *DEV_DOWN = "down";
constexpr const char *TEST_PROXY_HOST = "127.0.0.1";
constexpr const char *TEST_MAC_ADDRESS = "a0:0b:c1:d0:02:03";
constexpr uint16_t TEST_PROXY_PORT = 8080;
constexpr const char *IFACE = "iface0";
const int32_t FD = 5;
const int32_t SYSTEM_ABILITY_INVALID = 666;
constexpr uint16_t DEPENDENT_SERVICE_ALL = 0x0003;
const int32_t RET_ZERO = 0;
constexpr const char *SYS_PARAM_PERSIST_EDM_SET_ETHERNET_IP_DISABLE = "persist.edm.set_ethernet_ip_disable";

class MonitorInterfaceStateCallback : public InterfaceStateCallbackStub {
public:
    int32_t OnInterfaceAdded(const std::string &ifName) override
    {
        std::cout << "OnInterfaceAdded ifName: " << ifName << std::endl;
        return 0;
    }

    int32_t OnInterfaceRemoved(const std::string &ifName) override
    {
        std::cout << "OnInterfaceRemoved ifName: " << ifName << std::endl;
        return 0;
    }

    int32_t OnInterfaceChanged(const std::string &ifName, bool up) override
    {
        std::cout << "OnInterfaceChange ifName: " << ifName << ", state: " << up << std::endl;
        return 0;
    }

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        std::u16string descriptor = data.ReadInterfaceToken();
        if (descriptor != InterfaceStateCallback::GetDescriptor()) {
            NETMGR_EXT_LOG_E("OnRemoteRequest get descriptor error.");
            return NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH;
        }
        InterfaceStateCallback::Message msgCode = static_cast<InterfaceStateCallback::Message>(code);
        switch (msgCode) {
            case InterfaceStateCallback::Message::INTERFACE_STATE_ADD: {
                OnInterfaceAdded(data.ReadString());
                break;
            }
            case InterfaceStateCallback::Message::INTERFACE_STATE_REMOVE: {
                OnInterfaceRemoved(data.ReadString());
                break;
            }
            case InterfaceStateCallback::Message::INTERFACE_STATE_CHANGE: {
                OnInterfaceChanged(data.ReadString(), data.ReadBool());
                break;
            }
            default:
                return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
        return NETMANAGER_EXT_SUCCESS;
    }
};
} // namespace

class EthernetManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<InterfaceConfiguration> GetIfaceConfig();
    bool CheckIfaceUp(const std::string &iface);
};

void EthernetManagerTest::SetUpTestCase() {}

void EthernetManagerTest::TearDownTestCase() {}

void EthernetManagerTest::SetUp() {}

void EthernetManagerTest::TearDown() {}

sptr<InterfaceConfiguration> EthernetManagerTest::GetIfaceConfig()
{
    sptr<InterfaceConfiguration> ic = (std::make_unique<InterfaceConfiguration>()).release();
    if (!ic) {
        return ic;
    }
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = 0x01;
    ipv4Addr.prefixlen_ = 0x01;
    ipv4Addr.address_ = "172.17.5.234";
    ipv4Addr.netMask_ = "255.255.254.0";
    ipv4Addr.hostName_ = "netAddr";
    ic->ipStatic_.ipAddrList_.push_back(ipv4Addr);
    INetAddr route;
    route.type_ = INetAddr::IPV4;
    route.family_ = 0x01;
    route.prefixlen_ = 0x01;
    route.address_ = "0.0.0.0";
    route.netMask_ = "0.0.0.0";
    route.hostName_ = "netAddr";
    ic->ipStatic_.routeList_.push_back(route);
    INetAddr gateway;
    gateway.type_ = INetAddr::IPV4;
    gateway.family_ = 0x01;
    gateway.prefixlen_ = 0x01;
    gateway.address_ = "172.17.4.1";
    gateway.netMask_ = "0.0.0.0";
    gateway.hostName_ = "netAddr";
    ic->ipStatic_.gatewayList_.push_back(gateway);
    INetAddr netMask;
    netMask.type_ = INetAddr::IPV4;
    netMask.family_ = 0x01;
    netMask.address_ = "255.255.255.0";
    netMask.hostName_ = "netAddr";
    ic->ipStatic_.netMaskList_.push_back(netMask);
    ic->httpProxy_ = {TEST_PROXY_HOST, TEST_PROXY_PORT, {}};
    INetAddr dns1;
    dns1.type_ = INetAddr::IPV4;
    dns1.family_ = 0x01;
    dns1.address_ = "8.8.8.8";
    dns1.hostName_ = "netAddr";
    INetAddr dns2;
    dns2.type_ = INetAddr::IPV4;
    dns2.family_ = 0x01;
    dns2.address_ = "114.114.114.114";
    dns2.hostName_ = "netAddr";
    ic->ipStatic_.dnsServers_.push_back(dns1);
    ic->ipStatic_.dnsServers_.push_back(dns2);
    return ic;
}

bool EthernetManagerTest::CheckIfaceUp(const std::string &iface)
{
    NetManagerExtAccessToken token;
    int32_t activeStatus = 0;
    (void)DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(iface, activeStatus);
    return activeStatus == 1;
}

/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: Test EthernetManager OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, OnRemoteRequest, TestSize.Level1)
{
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = 0;
    ret = DelayedSingleton<MonitorInterfaceStateCallback>::GetInstance()->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: EthernetManager001
 * @tc.desc: Test EthernetManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager001, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ASSERT_EQ(DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(DEV_NAME, ic), NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: EthernetManager0011
 * @tc.desc: Test EthernetManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager0011, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    const char *DEV_NAME_1 = "eth3";
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(DEV_NAME_1, ic);
    ASSERT_EQ(ret, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);
}

/**
 * @tc.name: EthernetManager002
 * @tc.desc: Test EthernetManager GetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager002, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    sptr<InterfaceConfiguration> ic;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(DEV_NAME, ic);
    ASSERT_TRUE(ic != nullptr);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: EthernetManager0021
 * @tc.desc: Test EthernetManager GetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager0021, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    sptr<InterfaceConfiguration> ic;
    const char *DEV_NAME_1 = "eth3";
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(DEV_NAME_1, ic);
    ASSERT_FALSE(ic != nullptr);
    EXPECT_EQ(ret, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);
}

/**
 * @tc.name: EthernetManager003
 * @tc.desc: Test EthernetManager IsIfaceActive.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager003, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    int32_t activeStatus = -1;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(DEV_NAME, activeStatus);
    ASSERT_EQ(activeStatus, 1);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: EthernetManager0031
 * @tc.desc: Test EthernetManager IsIfaceActive.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager0031, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    int32_t activeStatus = -1;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive("eth3", activeStatus);
    ASSERT_NE(activeStatus, 1);
    EXPECT_EQ(ret, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);
}

/**
 * @tc.name: EthernetManager004
 * @tc.desc: Test EthernetManager GetAllActiveIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager004, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    std::vector<std::string> result;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetAllActiveIfaces(result);
    std::vector<std::string>::iterator it = std::find(result.begin(), result.end(), DEV_NAME);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ASSERT_TRUE(it != result.end());
}

/**
 * @tc.name: EthernetManager005
 * @tc.desc: Test EthernetManager GetMacAddress.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager005, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    std::vector<MacAddressInfo> mai;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetMacAddress(mai);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: ResetFactoryTest001
 * @tc.desc: Test EthernetManager ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, ResetFactoryTest001, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: ResetFactoryTest002
 * @tc.desc: Test EthernetManager ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, ResetFactoryTest002, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: ResetFactoryTest003
 * @tc.desc: Test EthernetManager ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, ResetFactoryTest003, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(EthernetManagerTest, EthernetManager006, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(DEV_NAME, cfg);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ASSERT_FALSE(cfg.ifName.empty());
    ASSERT_FALSE(cfg.hwAddr.empty());
}

HWTEST_F(EthernetManagerTest, OnInterfaceAddressUpdatedTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    EthernetManagement::DevInterfaceStateCallback devinterfacestatecallback(ethernetmanagement);
    std::string addr;
    std::string ifName;
    int flags = 0;
    int scope = 0;
    int ret = devinterfacestatecallback.OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, OnInterfaceAddressRemovedTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    EthernetManagement::DevInterfaceStateCallback devinterfacestatecallback(ethernetmanagement);
    std::string addr;
    std::string ifName;
    int flags = 0;
    int scope = 0;
    int ret = devinterfacestatecallback.OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, OnInterfaceAddedTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    EthernetManagement::DevInterfaceStateCallback devinterfacestatecallback(ethernetmanagement);
    std::string iface = "eth0";
    int ret = devinterfacestatecallback.OnInterfaceAdded(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, OnInterfaceRemovedTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    EthernetManagement::DevInterfaceStateCallback devinterfacestatecallback(ethernetmanagement);
    std::string iface = "eth0";
    int ret = devinterfacestatecallback.OnInterfaceRemoved(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, OnInterfaceChangedTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    EthernetManagement::DevInterfaceStateCallback devinterfacestatecallback(ethernetmanagement);
    std::string iface;
    int ret = devinterfacestatecallback.OnInterfaceChanged(iface, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, OnInterfaceLinkStateChangedTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    EthernetManagement::DevInterfaceStateCallback devinterfacestatecallback(ethernetmanagement);
    std::string ifName = "eth0";;
    int ret = devinterfacestatecallback.OnInterfaceLinkStateChanged(ifName, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, SetInterfaceConfig001, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = "eth0";
    config.hwAddr = "";
    config.ipv4Addr = "172.17.5.234";
    config.prefixLength = 24;
    config.flags.push_back("up");
    config.flags.push_back("broadcast");
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceConfig(DEV_NAME, config);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, SetInterfaceConfig002, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceDown(DEV_NAME);
    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = "eth0";
    config.hwAddr = "";
    config.ipv4Addr = "172.17.5.234";
    config.prefixLength = 24;
    config.flags.push_back("up");
    config.flags.push_back("broadcast");
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceConfig(DEV_NAME, config);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ASSERT_TRUE(result == 0);
}

HWTEST_F(EthernetManagerTest, EthernetManager007, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceUp(DEV_NAME);
    ASSERT_TRUE(result == 0);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    std::vector<std::string>().swap(cfg.flags);
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(DEV_NAME, cfg);
    ASSERT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(cfg.ifName, DEV_NAME);
    std::find(cfg.flags.begin(), cfg.flags.end(), DEV_DOWN);
    auto fit = std::find(cfg.flags.begin(), cfg.flags.end(), DEV_DOWN);
    ASSERT_NE(fit, cfg.flags.end());
    ASSERT_TRUE(*fit == DEV_DOWN);
}

HWTEST_F(EthernetManagerTest, EthernetManager008, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    int32_t result = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceUp(DEV_NAME);
    ASSERT_TRUE(result == 0);
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    std::vector<std::string>().swap(cfg.flags);
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->GetInterfaceConfig(DEV_NAME, cfg);
    ASSERT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(cfg.ifName, DEV_NAME);
    std::find(cfg.flags.begin(), cfg.flags.end(), DEV_UP);
    auto fit = std::find(cfg.flags.begin(), cfg.flags.end(), DEV_UP);
    ASSERT_NE(fit, cfg.flags.end());
    ASSERT_TRUE(*fit == DEV_UP);

    EthernetDhcpController ethernetDhcpController;
    bool bIpv6 = true;
    ethernetDhcpController.StartClient(IFACE, bIpv6);
    ethernetDhcpController.StopClient(IFACE, bIpv6);

    int32_t status = 0;
    std::string ifname = "";
    char *reason = nullptr;
    ethernetDhcpController.OnDhcpFailed(status, ifname, reason);
    DhcpResult dhcpResult;
    ethernetDhcpController.OnDhcpSuccess(IFACE, &dhcpResult);
    ethernetDhcpController.cbObject_ = nullptr;
    ethernetDhcpController.OnDhcpSuccess(IFACE, &dhcpResult);
}

HWTEST_F(EthernetManagerTest, EthernetManager009, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetService ethernetService;

    ethernetService.state_ = EthernetService::ServiceRunningState::STATE_RUNNING;
    ethernetService.OnStart();
    ethernetService.state_ = EthernetService::ServiceRunningState::STATE_STOPPED;
    ethernetService.OnStart();
    ethernetService.registerToService_ = false;
    bool initResult = ethernetService.Init();
    EXPECT_TRUE(initResult);
    ethernetService.OnStart();
    ethernetService.serviceComm_ = nullptr;
    initResult = ethernetService.Init();
    EXPECT_FALSE(initResult);
    ethernetService.OnStop();
    std::vector<std::u16string> args;
    std::u16string strU16 = u"ahaha";
    args.push_back(strU16);
    int32_t dumpRes = ethernetService.Dump(FD, args);
    EXPECT_NE(dumpRes, NETMANAGER_EXT_SUCCESS);
    dumpRes = ethernetService.Dump(FD, args);
    EXPECT_NE(dumpRes, NETMANAGER_EXT_SUCCESS);
    ethernetService.OnAddSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, DEV_NAME);
    ethernetService.OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, DEV_NAME);
    ethernetService.OnAddSystemAbility(SYSTEM_ABILITY_INVALID, DEV_NAME);
    ethernetService.dependentServiceState_ = DEPENDENT_SERVICE_ALL;
    ethernetService.OnAddSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, DEV_NAME);
}

HWTEST_F(EthernetManagerTest, EthernetManager010, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    std::string info = "info";
    EthernetManagement ethernetManagement;
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, false);
    EthernetDhcpCallback::DhcpResult dhcpResult;
    int32_t ret = ethernetManagement.UpdateDevInterfaceLinkInfo(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    sptr<InterfaceConfiguration> ic;
    ethernetManagement.GetDevInterfaceCfg(IFACE, ic);
    ethernetManagement.Init();
    ethernetManagement.StartSetDevUpThd();
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.DevInterfaceRemove(DEV_NAME);
    ethernetManagement.GetDumpInfo(info);
    EthernetManagement::DevInterfaceStateCallback devCallback(ethernetManagement);
    ret = devCallback.OnInterfaceAdded(IFACE);
    EXPECT_EQ(ret, RET_ZERO);
    ret = devCallback.OnInterfaceRemoved(IFACE);
    EXPECT_EQ(ret, RET_ZERO);
    ret = devCallback.OnInterfaceLinkStateChanged(IFACE, true);
    EXPECT_EQ(ret, RET_ZERO);
    ret = devCallback.OnInterfaceChanged(IFACE, true);
    EXPECT_EQ(ret, RET_ZERO);
}

HWTEST_F(EthernetManagerTest, EthernetManager011, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->RegisterIfacesStateChanged(nullptr);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager012, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<InterfaceStateCallback> interfaceCallback = new (std::nothrow) MonitorInterfaceStateCallback();
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->RegisterIfacesStateChanged(interfaceCallback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    ret = DelayedSingleton<EthernetClient>::GetInstance()->UnregisterIfacesStateChanged(interfaceCallback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager013, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->UnregisterIfacesStateChanged(nullptr);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager014, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    sptr<InterfaceStateCallback> interfaceCallback = new (std::nothrow) MonitorInterfaceStateCallback();
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->RegisterIfacesStateChanged(interfaceCallback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    sptr<InterfaceStateCallback> tmpCallback = new (std::nothrow) MonitorInterfaceStateCallback();
    ret = DelayedSingleton<EthernetClient>::GetInstance()->UnregisterIfacesStateChanged(tmpCallback);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
    ret = DelayedSingleton<EthernetClient>::GetInstance()->UnregisterIfacesStateChanged(interfaceCallback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager015, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::string dev = "eth0";
    ethernetManagement.UpdateInterfaceState(dev, true);

    std::vector<std::string> activeIfaces;
    int32_t ret = ethernetManagement.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager017, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    int32_t ret = ethernetManagement.ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager018, TestSize.Level1)
{
    int32_t activeStatus = -1;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(DEV_NAME, activeStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(EthernetManagerTest, EthernetManager019, TestSize.Level1)
{
    NetManagerExtNotSystemAccessToken token;
    int32_t activeStatus = -1;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(DEV_NAME, activeStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(EthernetManagerTest, EthernetManager020, TestSize.Level1)
{
    NoPermissionAccessToken token;
    int32_t activeStatus = -1;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->IsIfaceActive(DEV_NAME, activeStatus);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(EthernetManagerTest, EthernetManager021, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, false);
    EthernetDhcpCallback::DhcpResult dhcpResult;
    int32_t ret = ethernetManagement.UpdateDevInterfaceLinkInfo(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager022, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, false);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    std::string iface = "eth0";
    int32_t result = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result, ETHERNET_ERR_DEVICE_NOT_LINK);
}

HWTEST_F(EthernetManagerTest, EthernetManager023, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ic->mode_ = LAN_DHCP;
    std::string iface = "eth0";
    int32_t result = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result, NETMANAGER_ERR_INVALID_PARAMETER);
}

HWTEST_F(EthernetManagerTest, EthernetManager024, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ic->mode_ = DHCP;
    std::string iface = "eth0";
    int32_t result = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result, ETHERNET_ERR_USER_CONIFGURATION_WRITE_FAIL);
}

HWTEST_F(EthernetManagerTest, EthernetManager025, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ic->mode_ = DHCP;
    std::string iface = "eth0";
    int32_t result = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager026, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, false);
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.iface = "eth0";
    int32_t result = ethernetManagement.UpdateDevInterfaceLinkInfo(dhcpResult);
    EXPECT_EQ(result, ETHERNET_ERR_DEVICE_NOT_LINK);
}

HWTEST_F(EthernetManagerTest, EthernetManager027, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.iface = "eth0";
    int32_t result = ethernetManagement.UpdateDevInterfaceLinkInfo(dhcpResult);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager028, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(IFACE);
    ethernetManagement.UpdateInterfaceState(IFACE, false);
    sptr<InterfaceConfiguration> cfg;
    int32_t result = ethernetManagement.GetDevInterfaceCfg(IFACE, cfg);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager029, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(IFACE);
    ethernetManagement.UpdateInterfaceState(IFACE, true);
    sptr<InterfaceConfiguration> cfg;
    int32_t result = ethernetManagement.GetDevInterfaceCfg(IFACE, cfg);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager030, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    NetManagerExtAccessToken token;
    int32_t activeStatus = -1;
    int32_t ret = ethernetManagement.IsIfaceActive(DEV_NAME, activeStatus);
    ASSERT_EQ(activeStatus, 1);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager031, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::string dev = "eth0";
    ethernetManagement.DevInterfaceAdd(dev);
    ethernetManagement.UpdateInterfaceState(dev, true);
    std::vector<std::string> activeIfaces;
    int32_t ret = ethernetManagement.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager032, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.StartSetDevUpThd();
    EthernetManagement::DevInterfaceStateCallback devCallback(ethernetManagement);
    int32_t ret = devCallback.OnInterfaceAdded(IFACE);
    EXPECT_EQ(ret, RET_ZERO);
}

HWTEST_F(EthernetManagerTest, EthernetManager033, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::string dev = "eth0";
    ethernetManagement.DevInterfaceAdd(dev);
    ethernetManagement.UpdateInterfaceState(dev, true);
    std::vector<std::string> activeIfaces;
    int32_t ret = ethernetManagement.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManager034, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::string dev = "eth0";
    ethernetManagement.DevInterfaceAdd(dev);
    ethernetManagement.DevInterfaceAdd(dev);
    std::vector<std::string> activeIfaces;
    int32_t ret = ethernetManagement.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: EthernetManager035
 * @tc.desc: Test EthernetManagement::UpdateDevInterfaceCfg
 * Switch interface mode from hdcp to static when system param "persist.edm.set_ethernet_ip_disable" is true success
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager035, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ic->mode_ = STATIC;
    std::string iface = DEV_NAME;
    OHOS::system::SetParameter(SYS_PARAM_PERSIST_EDM_SET_ETHERNET_IP_DISABLE, "true");
    int32_t result = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
    OHOS::system::SetParameter(SYS_PARAM_PERSIST_EDM_SET_ETHERNET_IP_DISABLE, "false");
}

/**
 * @tc.name: EthernetManager036
 * @tc.desc: Test EthernetManagement::UpdateDevInterfaceCfg
 * Switch interface mode from static to dhcp when system param "persist.edm.set_ethernet_ip_disable" is true denied
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager036, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    sptr<InterfaceConfiguration> ic1 = GetIfaceConfig();
    ic1->mode_ = STATIC;
    std::string iface = DEV_NAME;
    int32_t result1 = ethernetManagement.UpdateDevInterfaceCfg(iface, ic1);
    EXPECT_EQ(result1, NETMANAGER_EXT_SUCCESS);

    OHOS::system::SetParameter(SYS_PARAM_PERSIST_EDM_SET_ETHERNET_IP_DISABLE, "true");
    sptr<InterfaceConfiguration> ic2 = GetIfaceConfig();
    ic2->mode_ = DHCP;
    int32_t result2 = ethernetManagement.UpdateDevInterfaceCfg(iface, ic2);
    EXPECT_EQ(result2, NETMANAGER_ERR_PERMISSION_DENIED);
    OHOS::system::SetParameter(SYS_PARAM_PERSIST_EDM_SET_ETHERNET_IP_DISABLE, "false");
}

/**
 * @tc.name: EthernetManager037
 * @tc.desc: Test EthernetManagement::UpdateDevInterfaceCfg
 * Modify interface when system param "persist.edm.set_ethernet_ip_disable" is true and interface mode is static denied
 * @tc.type: FUNC
 */
HWTEST_F(EthernetManagerTest, EthernetManager037, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    ethernetManagement.DevInterfaceAdd(DEV_NAME);
    ethernetManagement.UpdateInterfaceState(DEV_NAME, true);
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    ic->mode_ = STATIC;
    std::string iface = DEV_NAME;
    int32_t result1 = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result1, NETMANAGER_EXT_SUCCESS);

    OHOS::system::SetParameter(SYS_PARAM_PERSIST_EDM_SET_ETHERNET_IP_DISABLE, "true");
    int32_t result2 = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result2, NETMANAGER_ERR_PERMISSION_DENIED);
    OHOS::system::SetParameter(SYS_PARAM_PERSIST_EDM_SET_ETHERNET_IP_DISABLE, "false");
}


HWTEST_F(EthernetManagerTest, EthernetDhcpController001, TestSize.Level1)
{
    EthernetDhcpController dhcpController;
    sptr<EthernetDhcpCallback> callback = nullptr;
    dhcpController.RegisterDhcpCallback(callback);
    const std::string iface = "eth0";
    dhcpController.StartClient(iface, true);
    dhcpController.StopClient(iface, true);

    DhcpResult result;
    dhcpController.OnDhcpSuccess(iface, &result);

    EthernetDhcpController::EthernetDhcpControllerResultNotify ethernetDhcpControllerResultNotify;
    int status = 1;
    std::string ifname;
    std::string reason;
    ethernetDhcpControllerResultNotify.OnSuccess(status, ifname.c_str(), &result);
    ethernetDhcpControllerResultNotify.OnFailed(status, ifname.c_str(), reason.c_str());

    ethernetDhcpControllerResultNotify.OnSuccess(status, nullptr, &result);
    ethernetDhcpControllerResultNotify.OnSuccess(status, ifname.c_str(), nullptr);
    ethernetDhcpControllerResultNotify.SetEthernetDhcpController(nullptr);
    ethernetDhcpControllerResultNotify.OnSuccess(status, ifname.c_str(), &result);
    EXPECT_EQ(status, 1);
}

HWTEST_F(EthernetManagerTest, EthernetDhcpController002, TestSize.Level1)
{
    EthernetDhcpController dhcpController;
    sptr<EthernetDhcpCallback> callback;
    dhcpController.RegisterDhcpCallback(callback);
    const std::string iface = "eth0";
    dhcpController.StartClient(iface, true);
    dhcpController.StopClient(iface, true);
    DhcpResult result;
    dhcpController.OnDhcpSuccess(iface, &result);
    EXPECT_EQ(dhcpController.cbObject_, nullptr);
}

HWTEST_F(EthernetManagerTest, SetInterfaceUpTest001, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    std::string iface = "eth1";
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceUp(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, SetInterfaceDownTest001, TestSize.Level1)
{
    if (!CheckIfaceUp(DEV_NAME)) {
        return;
    }
    NetManagerExtAccessToken token;
    std::string iface = "eth1";
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->SetInterfaceDown(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManagerTestBranchTest001, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    NetsysControllerCallback::DhcpResult dhcpResult;
    EthernetManagement::DevInterfaceStateCallback devCallback(ethernetManagement);

    auto ret = devCallback.OnInterfaceAdded(IFACE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = devCallback.OnInterfaceRemoved(IFACE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = devCallback.OnRouteChanged(true, "", "", "");
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = devCallback.OnDhcpSuccess(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = devCallback.OnBandwidthReachedLimit("", IFACE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    EthernetDhcpCallback::DhcpResult dhcp;
    EthernetManagement::EhternetDhcpNotifyCallback ehternetDhcpNotifyCallback(ethernetManagement);
    ret = ehternetDhcpNotifyCallback.OnDhcpSuccess(dhcp);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetManagerTestBranchTest002, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    EthernetDhcpCallback::DhcpResult dhcpResult;

    sptr<InterfaceConfiguration> cfg = nullptr;
    auto ret = ethernetManagement.UpdateDevInterfaceCfg(IFACE, cfg);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);

    ret = ethernetManagement.UpdateDevInterfaceLinkInfo(dhcpResult);
    EXPECT_EQ(ret, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);
	
    IPSetMode origin = STATIC;
    IPSetMode input = DHCP;
    ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_TRUE(ret);

    origin = STATIC;
    input = LAN_STATIC;
    ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_FALSE(ret);

    origin = LAN_STATIC;
    input = DHCP;
    ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_FALSE(ret);

    origin = LAN_STATIC;
    input = LAN_DHCP;
    ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_TRUE(ret);

    ret = ethernetManagement.GetDevInterfaceCfg(IFACE, cfg);
    EXPECT_EQ(ret, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);

    sptr<DevInterfaceState> devState = new (std::nothrow) DevInterfaceState();
    if (devState != nullptr) {
        ethernetManagement.StartDhcpClient(DEV_NAME, devState);
        ethernetManagement.StopDhcpClient(DEV_NAME, devState);
    }
    ethernetManagement.DevInterfaceRemove(DEV_NAME);

    int32_t activeStatus = 0;
    ret = ethernetManagement.IsIfaceActive(IFACE, activeStatus);
    EXPECT_EQ(ret, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);
}

HWTEST_F(EthernetManagerTest, EthernetManagerBranchTest003, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    std::string iface = "";
    int32_t result = ethernetManagement.UpdateDevInterfaceCfg(iface, ic);
    EXPECT_EQ(result, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);

    std::string devName = "";
    ethernetManagement.DevInterfaceAdd(devName);
    bool ret = ethernetManagement.IsIfaceLinkUp(iface);
    EXPECT_FALSE(ret);
}

HWTEST_F(EthernetManagerTest, UpdateInterfaceStateTest001, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    ethernetManagement.devs_["test_dev"] = nullptr;
    ethernetManagement.UpdateInterfaceState("test_dev", true);
    EXPECT_NE(ethernetManagement.ethLanManageMent_, nullptr);
}

HWTEST_F(EthernetManagerTest, UpdateInterfaceStateTest002, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    DevInterfaceState devInterfaceState;
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    if (ifCfg != nullptr)
    {
        devInterfaceState.ifCfg_ = ifCfg;
    }
    devInterfaceState.ifCfg_->mode_ = DHCP;
    std::string dev = "eth0";
    devInterfaceState.dhcpReqState_ = false;
    ethernetManagement.UpdateInterfaceState(dev, false);
    devInterfaceState.ifCfg_->mode_ = LAN_DHCP;
    ethernetManagement.UpdateInterfaceState(dev, false);
    EXPECT_EQ(devInterfaceState.dhcpReqState_, false);
    EXPECT_NE(ethernetManagement.ethLanManageMent_, nullptr);
}

HWTEST_F(EthernetManagerTest, UpdateInterfaceStateTest003, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    DevInterfaceState devInterfaceState;
    std::string dev = "eth0";
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    if (ifCfg != nullptr)
    {
        devInterfaceState.ifCfg_ = ifCfg;
    }
    devInterfaceState.ifCfg_->mode_ = DHCP;
    devInterfaceState.dhcpReqState_ = true;
    ethernetManagement.UpdateInterfaceState(dev, false);
    devInterfaceState.ifCfg_->mode_ = LAN_DHCP;
    ethernetManagement.UpdateInterfaceState(dev, false);
    EXPECT_NE(ethernetManagement.ethLanManageMent_, nullptr);
    EXPECT_EQ(devInterfaceState.dhcpReqState_, true);
}

HWTEST_F(EthernetManagerTest, UpdateInterfaceStateTest004, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    DevInterfaceState devInterfaceState;
    std::string dev = "eth0";
    sptr<InterfaceConfiguration> ifCfg = new (std::nothrow) InterfaceConfiguration();
    if (ifCfg != nullptr)
    {
        devInterfaceState.ifCfg_ = ifCfg;
    }
    devInterfaceState.ifCfg_->mode_ = STATIC;
    devInterfaceState.dhcpReqState_ = false;
    ethernetManagement.UpdateInterfaceState(dev, true);

    devInterfaceState.ifCfg_->mode_ = STATIC;
    devInterfaceState.dhcpReqState_ = true;
    ethernetManagement.UpdateInterfaceState(dev, true);

    devInterfaceState.ifCfg_->mode_ = LAN_DHCP;
    devInterfaceState.dhcpReqState_ = true;
    ethernetManagement.UpdateInterfaceState(dev, true);

    EXPECT_NE(ethernetManagement.ethLanManageMent_, nullptr);
    EXPECT_EQ(devInterfaceState.dhcpReqState_, true);
}

HWTEST_F(EthernetManagerTest, GetMacAddressTest001, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::vector<MacAddressInfo> macAddrList;
    int32_t ret = ethernetManagement.GetMacAddress(macAddrList);
    EXPECT_EQ(ret, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);
}

HWTEST_F(EthernetManagerTest, GetMacAddrTest001, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    string macAddr = ethernetManagement.GetMacAddr("eth0");
    EXPECT_TRUE(macAddr.empty());
}

HWTEST_F(EthernetManagerTest, HwAddrToStrTest001, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    char hwaddr[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    string macAddr = ethernetManagement.HwAddrToStr(hwaddr);
    EXPECT_EQ(macAddr, "00:11:22:33:44:55");
    macAddr = ethernetManagement.HwAddrToStr(nullptr);
    EXPECT_TRUE(macAddr.empty());
}

HWTEST_F(EthernetManagerTest, ModeInputCheckTest, TestSize.Level1)
{
    EthernetManagement ethernetManagement;

    IPSetMode origin = DHCP;
    IPSetMode input = LAN_STATIC;
    bool ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_FALSE(ret);

    origin = DHCP;
    input = LAN_DHCP;
    ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_FALSE(ret);

    origin = LAN_DHCP;
    input = STATIC;
    ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_FALSE(ret);

    origin = LAN_DHCP;
    input = DHCP;
    ret = ethernetManagement.ModeInputCheck(origin, input);
    EXPECT_FALSE(ret);
}

HWTEST_F(EthernetManagerTest, DevInterfaceRemoveTest001, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::string dev = "eth0";
    ethernetManagement.DevInterfaceAdd(dev);
    ethernetManagement.DevInterfaceRemove(dev);
    std::vector<std::string> activeIfaces;
    int32_t ret = ethernetManagement.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(std::count(activeIfaces.begin(), activeIfaces.end(), dev));
}

HWTEST_F(EthernetManagerTest, DevInterfaceRemoveTest002, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::string dev = "eth0";
    ethernetManagement.DevInterfaceAdd(dev);
    auto fitDev = ethernetManagement.devs_.find(dev);
    if (fitDev != ethernetManagement.devs_.end())
    {
        fitDev->second = nullptr;
    }
    ethernetManagement.DevInterfaceRemove(dev);
    std::vector<std::string> activeIfaces;
    int32_t ret = ethernetManagement.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(std::count(activeIfaces.begin(), activeIfaces.end(), dev));
}

HWTEST_F(EthernetManagerTest, DevInterfaceAddTest001, TestSize.Level1)
{
    EthernetManagement ethernetManagement;
    std::string dev = "eth0";
    sptr<InterfaceConfiguration> cfg = new InterfaceConfiguration();
    cfg->mode_ = LAN_STATIC;
    ethernetManagement.devCfgs_[dev] = cfg;
    ethernetManagement.DevInterfaceAdd(dev);
    std::vector<std::string> activeIfaces;
    int32_t ret = ethernetManagement.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    cfg->mode_ = LAN_DHCP;
    ethernetManagement.devCfgs_[dev] = cfg;
    ethernetManagement.DevInterfaceAdd(dev);
    std::vector<std::string> activeIfaces1;
    ret = ethernetManagement.GetAllActiveIfaces(activeIfaces1);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    cfg->mode_ = STATIC;
    ethernetManagement.devCfgs_[dev] = cfg;
    ethernetManagement.DevInterfaceAdd(dev);
    std::vector<std::string> activeIfaces2;
    ret = ethernetManagement.GetAllActiveIfaces(activeIfaces2);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, EthernetDhcpControllerOnSuccessTest001, TestSize.Level1)
{
    EthernetDhcpController dhcpController;
    sptr<EthernetDhcpCallback> callback = nullptr;
    dhcpController.RegisterDhcpCallback(callback);
    const std::string iface = "eth0";
    dhcpController.StartClient(iface, true);
    dhcpController.StopClient(iface, true);

    DhcpResult result;
    dhcpController.OnDhcpSuccess(iface, &result);

    EthernetDhcpController::EthernetDhcpControllerResultNotify ethernetDhcpControllerResultNotify;
    int status = 1;
    const char *ifname = nullptr;
    ethernetDhcpControllerResultNotify.OnSuccess(status, ifname, &result);
    EXPECT_EQ(status, 1);
}

HWTEST_F(EthernetManagerTest, EthernetDhcpControllerOnSuccessTest002, TestSize.Level1)
{
    EthernetDhcpController::EthernetDhcpControllerResultNotify ethernetDhcpControllerResultNotify;
    int status = 1;
    const char *ifname = nullptr;
    DhcpResult *result = nullptr;
    ethernetDhcpControllerResultNotify.OnSuccess(status, ifname, result);
    EXPECT_EQ(status, 1);
}

HWTEST_F(EthernetManagerTest, EthernetDhcpControllerOnSuccessTest003, TestSize.Level1)
{
    EthernetDhcpController::EthernetDhcpControllerResultNotify ethernetDhcpControllerResultNotify;
    int status = 1;
    const char *ifname = "eth0";
    DhcpResult *result = nullptr;
    ethernetDhcpControllerResultNotify.OnSuccess(status, ifname, result);
    EXPECT_EQ(status, 1);
}

HWTEST_F(EthernetManagerTest, EthernetDhcpControllerOnSuccessTest004, TestSize.Level1)
{
    EthernetDhcpController dhcpController;
    sptr<EthernetDhcpCallback> callback = nullptr;
    dhcpController.RegisterDhcpCallback(callback);
    const std::string iface = "eth0";
    dhcpController.StartClient(iface, true);
    dhcpController.StopClient(iface, true);

    DhcpResult result;
    dhcpController.OnDhcpSuccess(iface, &result);

    EthernetDhcpController::EthernetDhcpControllerResultNotify ethernetDhcpControllerResultNotify;
    ethernetDhcpControllerResultNotify.ethDhcpController_ = nullptr;
    int status = 1;
    const char *ifname = "eth0";
    ethernetDhcpControllerResultNotify.OnSuccess(status, ifname, &result);
    EXPECT_EQ(status, 1);
}

class MyEthernetDhcpCallback : public EthernetDhcpCallback {
public:
    int32_t OnDhcpSuccess(EthernetDhcpCallback::DhcpResult &dhcpResult) override
    {
        std::cout << "DHCP Success: " << dhcpResult.ipAddr << std::endl;
        return 0;
    }
};

HWTEST_F(EthernetManagerTest, EthernetDhcpController003, TestSize.Level1)
{
    EthernetDhcpController dhcpController;
    sptr<EthernetDhcpCallback> callback = new MyEthernetDhcpCallback();
    dhcpController.RegisterDhcpCallback(callback);
    const std::string iface = "eth0";
    dhcpController.StartClient(iface, true);
    dhcpController.StopClient(iface, true);
    DhcpResult result;
    dhcpController.OnDhcpSuccess(iface, &result);
    EXPECT_NE(dhcpController.cbObject_, nullptr);
}

HWTEST_F(EthernetManagerTest, EthernetDhcpController004, TestSize.Level1)
{
    EthernetDhcpController dhcpController;
    EthernetDhcpController::EthernetDhcpControllerResultNotify ethernetDhcpControllerResultNotify;
    DhcpResult result;
    char *ifname;
    std::string ifname1;
    ethernetDhcpControllerResultNotify.OnSuccess(1, ifname, &result);
    EthernetDhcpController *ethDhcpController;
    ethDhcpController = nullptr;
    ethernetDhcpControllerResultNotify.SetEthernetDhcpController(ethDhcpController);
    ethernetDhcpControllerResultNotify.OnSuccess(1, ifname1.c_str(), &result);
    EXPECT_EQ(dhcpController.cbObject_, nullptr);
}

HWTEST_F(EthernetManagerTest, OnStartTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    ethernetservice.state_ = EthernetService::ServiceRunningState::STATE_RUNNING;
    ethernetservice.OnStart();
    ethernetservice.OnStart();
    EXPECT_EQ(ethernetservice.state_, EthernetService::ServiceRunningState::STATE_RUNNING);
}

HWTEST_F(EthernetManagerTest, UpdateInterfaceStateTest005, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    std::string dev = "";
    ethernetmanagement.UpdateInterfaceState(dev, true);
    ethernetmanagement.UpdateInterfaceState(dev, false);
    EXPECT_EQ(dev, "");
}

HWTEST_F(EthernetManagerTest, GetMacAddressTest002, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    std::vector<MacAddressInfo> macAddrList = {};
    EXPECT_NE(ethernetmanagement.GetMacAddress(macAddrList), NETMANAGER_EXT_SUCCESS);
    MacAddressInfo macaddressinfo;
    macaddressinfo.iface_ = "123";
    macaddressinfo.macAddress_ = "123";
    macAddrList = {macaddressinfo};
    EXPECT_EQ(ethernetmanagement.GetMacAddress(macAddrList), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, UpdateDevInterfaceCfgTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    std::string iface = "123";
    sptr<InterfaceConfiguration> cfg = new InterfaceConfiguration();
    ethernetmanagement.DevInterfaceAdd(iface);
    EXPECT_EQ(ethernetmanagement.UpdateDevInterfaceCfg(iface, cfg), ETHERNET_ERR_DEVICE_NOT_LINK);
    ethernetmanagement.UpdateInterfaceState(iface, true);
    EXPECT_EQ(ethernetmanagement.UpdateDevInterfaceCfg(iface, cfg), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EthernetManagerTest, UpdateDevInterfaceLinkInfoTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    EthernetDhcpCallback::DhcpResult dhcpResult;
    dhcpResult.iface = "123";
    ethernetmanagement.DevInterfaceAdd(dhcpResult.iface);
    EXPECT_EQ(ethernetmanagement.UpdateDevInterfaceLinkInfo(dhcpResult), ETHERNET_ERR_DEVICE_NOT_LINK);
    dhcpResult.iface = "1";
    ethernetmanagement.DevInterfaceAdd(dhcpResult.iface);
    EXPECT_EQ(ethernetmanagement.UpdateDevInterfaceLinkInfo(dhcpResult), ETHERNET_ERR_DEVICE_NOT_LINK);
    ethernetmanagement.UpdateInterfaceState(dhcpResult.iface, true);
    EXPECT_EQ(ethernetmanagement.UpdateDevInterfaceLinkInfo(dhcpResult), ETHERNET_ERR_CONVERT_CONFIGURATINO_FAIL);
}

HWTEST_F(EthernetManagerTest, GetDevInterfaceCfgTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    std::string iface = "123";
    sptr<InterfaceConfiguration> ifaceConfig = new InterfaceConfiguration();
    EXPECT_NE(ethernetmanagement.GetDevInterfaceCfg(iface, ifaceConfig), ETHERNET_ERR_INIT_FAIL);
}

HWTEST_F(EthernetManagerTest, IsIfaceActiveTest001, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    std::string iface = "123";
    int32_t activeStatus = 123;
    EXPECT_NE(ethernetmanagement.IsIfaceActive(iface, activeStatus), ETHERNET_ERR_INIT_FAIL);
}

HWTEST_F(EthernetManagerTest, DevInterfaceRemoveTest003, TestSize.Level1)
{
    EthernetManagement ethernetmanagement;
    std::string devName = "123";
    ethernetmanagement.DevInterfaceRemove(devName);
    EXPECT_EQ(devName, "123");
}
 
#ifdef NET_EXTENSIBLE_AUTHENTICATION
 
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
 
class EapPostbackCallbackTest : public NetEapPostbackCallbackStub {
public:
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override
    {
        return 0;
    }
};
 
HWTEST_F(EthernetManagerTest, RegCustomEapHandlerTest, TestSize.Level1)
{
    const NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    sptr<NetRegisterEapCallbackTest> netRegisterEapCallback = new (std::nothrow) NetRegisterEapCallbackTest();
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->RegisterCustomEapCallback(
        netType, netRegisterEapCallback);
    sptr<EapPostbackCallbackTest> eapPostBackCallback = new (std::nothrow) EapPostbackCallbackTest();
    ret = DelayedSingleton<EthernetClient>::GetInstance()->RegCustomEapHandler(netType, regCmd, eapPostBackCallback);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}
 
HWTEST_F(EthernetManagerTest, ReplyCustomEapDataTest, TestSize.Level1)
{
    NetType netType = NetType::WLAN0;
    std::string regCmd = "2:277:278";
    int result = 1;
    sptr<EapData> eapData = new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 13;
    eapData->msgId = 55;
    eapData->bufferLen = 4;
    std::vector<uint8_t> tmp = {0x11, 0x12};
    eapData->eapBuffer = tmp;
 
    sptr<EapPostbackCallbackTest> eapPostBackCallback = new (std::nothrow) EapPostbackCallbackTest();
    sptr<NetRegisterEapCallbackTest> netRegisterEapCallback = new (std::nothrow) NetRegisterEapCallbackTest();
 
    auto ret = DelayedSingleton<EthernetClient>::GetInstance()->ReplyCustomEapData(result, eapData);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}
 
HWTEST_F(EthernetManagerTest, RegisterCustomEapCallbackTest, TestSize.Level1)
{
    const NetType netType = NetType::WLAN0;
    sptr<NetRegisterEapCallbackTest> netRegisterEapCallback = new (std::nothrow) NetRegisterEapCallbackTest();
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->RegisterCustomEapCallback(
        netType, netRegisterEapCallback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
 
HWTEST_F(EthernetManagerTest, UnRegisterCustomEapCallbackTest, TestSize.Level1)
{
    const NetType netType = NetType::WLAN0;
    sptr<NetRegisterEapCallbackTest> netRegisterEapCallback = new (std::nothrow) NetRegisterEapCallbackTest();
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->UnRegisterCustomEapCallback(
        netType, netRegisterEapCallback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
 
HWTEST_F(EthernetManagerTest, NotifyWpaEapInterceptInfoTest, TestSize.Level1)
{
    const NetType netType = NetType::WLAN0;
    sptr<EapData> eapData =new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 13;
    eapData->msgId = 55;
    eapData->bufferLen = 4;
    std::vector<uint8_t> tmp = {0x11, 0x12};
    eapData->eapBuffer = tmp;
    int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->NotifyWpaEapInterceptInfo(netType, eapData);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
#endif

HWTEST_F(EthernetManagerTest, EthernetManagerGetDeviceInfoTest01, TestSize.Level1)
{
    std::string eth0 = "eth0";
    std::string testPath1 = "testPath";
    std::string testPath2 = "testPath/testPath2";
    std::string testPath3 = "/data/service/el1/public/dev_info";
    std::ofstream outfile;
    outfile.open(testPath3);
    if (outfile.is_open()) {
        outfile << "YT8521 Ethernet,011a,YT,1000";
        outfile.close();
    }
 
    std::vector<EthernetDeviceInfo> deviceInfoList;
    EthernetManagement ethernetManagement;
    ethernetManagement.devs_.clear();
    ethernetManagement.GetDeviceInformation(deviceInfoList);
    EXPECT_EQ(deviceInfoList.size(), 0);
    EthernetDeviceInfo tmp;
    deviceInfoList.push_back(tmp);
    ethernetManagement.GetDeviceInformation(deviceInfoList);
    EXPECT_GE(deviceInfoList.size(), 0);
 
    ethernetManagement.GetUsbEthDeviceInfo(eth0, testPath1, deviceInfoList);
    ethernetManagement.GetUsbEthDeviceInfo(eth0, testPath2, deviceInfoList);
    EXPECT_GE(deviceInfoList.size(), 0);
 
    ethernetManagement.GetPciEthDeviceInfo(eth0, testPath1, deviceInfoList);
    ethernetManagement.GetPciEthDeviceInfo(eth0, testPath2, deviceInfoList);
    ethernetManagement.GetUsbEthDeviceInfo(eth0, testPath3, deviceInfoList);
    EXPECT_GE(deviceInfoList.size(), 0);
 
    std::string value;
    ethernetManagement.GetSysNodeValue(testPath2, value);
    ethernetManagement.GetSysNodeValue(testPath3, value);
    EXPECT_GE(value.length(), 0);
}

} // namespace NetManagerStandard
} // namespace OHOS
