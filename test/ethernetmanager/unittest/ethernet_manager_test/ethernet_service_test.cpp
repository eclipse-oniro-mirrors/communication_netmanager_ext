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

#include "ethernet_client.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "interface_configuration.h"
#include "interface_type.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "static_configuration.h"

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
constexpr const char *IFACE_NAME = "wlan0";
} // namespace

class EtherNetServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    sptr<InterfaceConfiguration> GetIfaceConfig();
    void SetUp();
    void TearDown();
};

sptr<InterfaceConfiguration> EtherNetServiceTest::GetIfaceConfig()
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

void EtherNetServiceTest::SetUpTestCase() {}

void EtherNetServiceTest::TearDownTestCase() {}

void EtherNetServiceTest::SetUp() {}

void EtherNetServiceTest::TearDown() {}

HWTEST_F(EtherNetServiceTest, OnInterfaceAddressUpdatedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string addr;
    std::string ifName;
    int flags = 0;
    int scope = 0;
    int ret = globalinterfacestatecallback.OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceAddressRemovedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string addr;
    std::string ifName;
    int flags = 0;
    int scope = 0;
    int ret = globalinterfacestatecallback.OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceAddedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string iface;
    int ret = globalinterfacestatecallback.OnInterfaceAdded(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceRemovedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string iface;
    int ret = globalinterfacestatecallback.OnInterfaceRemoved(iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceChangedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string iface;
    int ret = globalinterfacestatecallback.OnInterfaceChanged(iface, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnInterfaceLinkStateChangedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string ifName;
    int ret = globalinterfacestatecallback.OnInterfaceLinkStateChanged(ifName, true);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnRouteChangedTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    bool updated = true;
    std::string route;
    std::string gateway;
    std::string ifName;
    int ret = globalinterfacestatecallback.OnRouteChanged(updated, route, gateway, ifName);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnDhcpSuccessTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    NetsysControllerCallback::DhcpResult dhcpResult;
    int ret = globalinterfacestatecallback.OnDhcpSuccess(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, OnBandwidthReachedLimitTest001, TestSize.Level1)
{
    EthernetService ethernetservice;
    EthernetService::GlobalInterfaceStateCallback globalinterfacestatecallback(ethernetservice);
    std::string limitName;
    std::string iface;
    int ret = globalinterfacestatecallback.OnBandwidthReachedLimit(limitName, iface);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetIfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    int ret = ethernetService.SetIfaceConfig(DEV_NAME, ic);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(EtherNetServiceTest, GetIfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    std::string iface;
    sptr<InterfaceConfiguration> ifaceConfig = GetIfaceConfig();
    int ret = ethernetService.GetIfaceConfig(iface, ifaceConfig);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

HWTEST_F(EtherNetServiceTest, IsIfaceActiveTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceConfiguration> ic = GetIfaceConfig();
    int32_t ret = ethernetService.SetIfaceConfig(DEV_NAME, ic);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, GetAllActiveIfacesTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    std::vector<std::string> result;
    int32_t ret = ethernetService.GetAllActiveIfaces(result);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, ResetFactoryTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    int32_t ret = ethernetService.ResetFactory();
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, RegisterIfacesStateChangedTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceStateCallback> callback;
    int32_t ret = ethernetService.RegisterIfacesStateChanged(callback);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, UnregisterIfacesStateChangedTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    sptr<InterfaceStateCallback> callback;
    int32_t ret = ethernetService.UnregisterIfacesStateChanged(callback);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetInterfaceUpTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    int32_t ret = ethernetService.SetInterfaceUp(DEV_NAME);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetInterfaceDownTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    int32_t ret = ethernetService.SetInterfaceDown(DEV_NAME);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, GetInterfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    int32_t ret = ethernetService.GetInterfaceConfig(DEV_NAME, cfg);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, SetInterfaceConfigTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    std::string deviceId;
    int32_t systemAbilityId = 0;
    ethernetService.OnAddSystemAbility(systemAbilityId, deviceId);
    ethernetService.OnAddSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, deviceId);
    ethernetService.OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, deviceId);

    OHOS::nmd::InterfaceConfigurationParcel config;
    config.ifName = "eth0";
    config.hwAddr = "";
    config.ipv4Addr = "172.17.5.234";
    config.prefixLength = 24;
    config.flags.push_back("up");
    config.flags.push_back("broadcast");
    int32_t ret = ethernetService.SetInterfaceConfig(DEV_NAME, config);
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, EthernetServiceCommonTest001, TestSize.Level1)
{
    sptr<EthernetServiceCommon> serviceComm_ = new (std::nothrow) EthernetServiceCommon();
    if (serviceComm_ == nullptr) {
        NETMGR_EXT_LOG_E("serviceComm_ is nullptr");
        return;
    }
    auto ret = serviceComm_->ResetEthernetFactory();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_PERMISSION_DENIED);

    NetManagerExtAccessToken token;
    ret = serviceComm_->ResetEthernetFactory();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(EtherNetServiceTest, EthernetServiceBranchTest001, TestSize.Level1)
{
    EthernetService ethernetService;
    bool ret = ethernetService.Init();
    EXPECT_FALSE(ret);

    ethernetService.OnStop();

    NetManagerExtNotSystemAccessToken token;
    int32_t result = ethernetService.ResetFactory();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);

    ethernetService.InitManagement();

    std::string iface = "";
    sptr<InterfaceConfiguration> ic = nullptr;
    result = ethernetService.SetIfaceConfig(iface, ic);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);

    result = ethernetService.GetIfaceConfig(iface, ic);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);

    int32_t activeStatus = 0;
    result = ethernetService.IsIfaceActive(iface, activeStatus);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);

    std::vector<std::string> activeIfaces;
    result = ethernetService.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);

    result = ethernetService.ResetFactory();
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

HWTEST_F(EtherNetServiceTest, EthernetServiceBranchTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    EthernetService ethernetService;
    ethernetService.InitManagement();

    sptr<InterfaceStateCallback> callback = nullptr;
    auto result = ethernetService.RegisterIfacesStateChanged(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    result = ethernetService.UnregisterMonitorIfaceCallbackAsync(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_OPERATION_FAILED);

    OHOS::nmd::InterfaceConfigurationParcel config;
    result = ethernetService.SetInterfaceConfig(IFACE_NAME, config);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    result = ethernetService.GetInterfaceConfig(IFACE_NAME, config);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    result = ethernetService.Dump(fd, args);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    std::string iface = "";
    sptr<InterfaceConfiguration> ic = nullptr;
    result = ethernetService.SetIfaceConfig(IFACE_NAME, ic);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);

    result = ethernetService.GetIfaceConfig(IFACE_NAME, ic);
    EXPECT_EQ(result, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);

    int32_t activeStatus = 0;
    result = ethernetService.IsIfaceActive(iface, activeStatus);
    EXPECT_EQ(result, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);

    result = ethernetService.SetInterfaceDown(IFACE_NAME);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    OHOS::nmd::InterfaceConfigurationParcel cfg;
    result = ethernetService.SetInterfaceConfig(IFACE_NAME, config);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    result = ethernetService.GetInterfaceConfig(IFACE_NAME, cfg);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    std::vector<std::string> activeIfaces;
    result = ethernetService.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);

    result = ethernetService.SetInterfaceUp(IFACE_NAME);
    EXPECT_EQ(result, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EtherNetServiceTest, EthernetServiceBranchTest003, TestSize.Level1)
{
    EthernetService ethernetService;
    bool ret = ethernetService.Init();
    EXPECT_FALSE(ret);

    ethernetService.OnStart();
    ethernetService.OnStop();

    NetManagerExtAccessToken token;
    int32_t fd = 0;
    std::vector<std::u16string> args;
    int32_t result = ethernetService.Dump(fd, args);
    EXPECT_EQ(result, 0);

    std::string iface = "";
    sptr<InterfaceConfiguration> ic = nullptr;

    result = ethernetService.SetIfaceConfig(iface, ic);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);

    result = ethernetService.GetIfaceConfig(iface, ic);
    EXPECT_EQ(result, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);

    int32_t activeStatus = 0;
    result = ethernetService.IsIfaceActive(iface, activeStatus);
    EXPECT_EQ(result, ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST);

    std::vector<std::string> activeIfaces;
    result = ethernetService.GetAllActiveIfaces(activeIfaces);
    EXPECT_EQ(result, 0);
}

HWTEST_F(EtherNetServiceTest, EthernetServiceBranchTest004, TestSize.Level1)
{
    EthernetService ethernetService;
    NoPermissionAccessToken token;
    int32_t activeStatus = 0;
    std::string iface = "";
    int32_t result = ethernetService.IsIfaceActive(iface, activeStatus);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);

    sptr<InterfaceStateCallback> callback = nullptr;
    result = ethernetService.RegisterIfacesStateChanged(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    result = ethernetService.UnregisterIfacesStateChanged(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PARAMETER_ERROR);

    result = ethernetService.RegisterMonitorIfaceCallbackAsync(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_OPERATION_FAILED);

    result = ethernetService.UnregisterMonitorIfaceCallbackAsync(callback);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_OPERATION_FAILED);
}
} // namespace NetManagerStandard
} // namespace OHOS
