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
#include <sys/socket.h>

#include "accesstoken_kit.h"
#include "extended_vpn_ctl.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "vpn_config.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "networkvpn_service.h"
#include "vpn_event_callback_stub.h"
#include "system_ability_definition.h"
#include "inet_addr.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

} // namespace

class NetworkVpnServiceExtTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = DelayedSingleton<NetworkVpnService>::GetInstance();
};

void NetworkVpnServiceExtTest::SetUpTestCase()
{
    instance_->OnStart();
}

void NetworkVpnServiceExtTest::TearDownTestCase()
{
    instance_->OnStop();
}

void NetworkVpnServiceExtTest::SetUp() {}

void NetworkVpnServiceExtTest::TearDown() {}


Route GetRouteInfo()
{
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNICAST;
    route.hasGateway_ = true;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    route.gateway_.type_ = INetAddr::IPV4;
    route.gateway_.family_ = AF_INET;
    route.gateway_.prefixlen_ = 0x18;
    route.gateway_.address_ = "192.168.2.1";
    route.gateway_.netMask_ = "255.255.255.0";
    route.gateway_.hostName_ = "netAddr";
    return route;
}


HWTEST_F(NetworkVpnServiceExtTest, ConvertNetAddrToConfigTest001, TestSize.Level1)
{
    INetAddr sourAddr;
    INetAddr destAddr;
    sourAddr.type_ = INetAddr::IPV4;
    sourAddr.family_ = AF_INET;
    sourAddr.prefixlen_ = 0x18;
    sourAddr.address_ = "192.168.2.10";
    sourAddr.netMask_ = "255.255.255.0";
    sourAddr.hostName_ = "netAddr";
    sourAddr.port_ = 80;
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        instance_->ConvertNetAddrToJson(sourAddr, json);
        instance_->ConvertNetAddrToConfig(destAddr, json);
        cJSON_Delete(json);
        json = nullptr;
        EXPECT_EQ(destAddr.family_, AF_INET);
    }
}

HWTEST_F(NetworkVpnServiceExtTest, ConvertNetAddrToConfigTest002, TestSize.Level1)
{
    INetAddr destAddr;
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        instance_->ConvertNetAddrToConfig(destAddr, json);
        cJSON_Delete(json);
        json = nullptr;
    }

    EXPECT_EQ(destAddr.port_, 0);
}

HWTEST_F(NetworkVpnServiceExtTest, ConvertNetAddrToConfigTest003, TestSize.Level1)
{
    INetAddr destAddr;
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        cJSON_AddItemToObject(json, "type", cJSON_CreateString("ss"));
        cJSON_AddItemToObject(json, "family", cJSON_CreateString("ss"));
        cJSON_AddItemToObject(json, "prefixlen", cJSON_CreateString("ss"));
        cJSON_AddItemToObject(json, "port", cJSON_CreateString("ss"));
        cJSON_AddItemToObject(json, "address", cJSON_CreateNumber(1));
        cJSON_AddItemToObject(json, "netMask", cJSON_CreateNumber(1));
        cJSON_AddItemToObject(json, "hostName", cJSON_CreateNumber(1));
        instance_->ConvertNetAddrToConfig(destAddr, json);
        cJSON_Delete(json);
        json = nullptr;
    }

    EXPECT_EQ(destAddr.port_, 0);
}

HWTEST_F(NetworkVpnServiceExtTest, ConvertRouteToConfig001, TestSize.Level1)
{
    Route destRoute;
    Route mem = GetRouteInfo();
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        cJSON_AddItemToObject(json, "iface", cJSON_CreateString(mem.iface_.c_str()));
        cJSON *jDestination = cJSON_CreateObject();
        instance_->ConvertNetAddrToJson(mem.destination_, jDestination);
        cJSON_AddItemToObject(json, "destination", jDestination);
        cJSON *jGateway = cJSON_CreateObject();
        instance_->ConvertNetAddrToJson(mem.gateway_, jGateway);
        cJSON_AddItemToObject(json, "gateway", jGateway);
        cJSON_AddItemToObject(json, "rtnType", cJSON_CreateNumber(mem.rtnType_));
        cJSON_AddItemToObject(json, "mtu", cJSON_CreateNumber(mem.mtu_));
        cJSON_AddItemToObject(json, "isHost", cJSON_CreateBool(mem.isHost_));
        cJSON_AddItemToObject(json, "hasGateway", cJSON_CreateBool(mem.hasGateway_));
        cJSON_AddItemToObject(json, "isDefaultRoute", cJSON_CreateBool(mem.isDefaultRoute_));
        instance_->ConvertRouteToConfig(destRoute, json);
        cJSON_Delete(json);
        json = nullptr;
        EXPECT_STREQ(destRoute.iface_.c_str(), "eth0");
    }
}
HWTEST_F(NetworkVpnServiceExtTest, ConvertRouteToConfig002, TestSize.Level1)
{
    Route destRoute;
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        instance_->ConvertRouteToConfig(destRoute, json);
        cJSON_Delete(json);
        json = nullptr;
    }
    EXPECT_EQ(destRoute.rtnType_, RTN_UNICAST);
}

HWTEST_F(NetworkVpnServiceExtTest, ConvertRouteToConfig003, TestSize.Level1)
{
    Route destRoute;
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        cJSON_AddItemToObject(json, "iface", cJSON_CreateNumber(1));
        cJSON_AddItemToObject(json, "destination", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "gateway", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "rtnType", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "mtu", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "isHost", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "hasGateway", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "isDefaultRoute", cJSON_CreateString("s"));
        instance_->ConvertRouteToConfig(destRoute, json);
        cJSON_Delete(json);
        json = nullptr;
    }
    EXPECT_EQ(destRoute.rtnType_, RTN_UNICAST);
}

HWTEST_F(NetworkVpnServiceExtTest, ParseJsonToConfig001, TestSize.Level1)
{
    std::string josnStr;
    sptr<VpnConfig> vpnCfg = new VpnConfig();
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        cJSON_AddItemToObject(json, "test", cJSON_CreateNumber(1));
        char *str = cJSON_Print(json);
        if (str != nullptr) {
            josnStr = str;
            free(str);
            str = nullptr;
            instance_->ParseJsonToConfig(vpnCfg, josnStr);
        }
        cJSON_Delete(json);
        json = nullptr;
    }
    EXPECT_EQ(vpnCfg->mtu_, 0);
}

HWTEST_F(NetworkVpnServiceExtTest, ParseJsonToConfig002, TestSize.Level1)
{
    std::string josnStr;
    sptr<VpnConfig> vpnCfg = new VpnConfig();
    cJSON *json = cJSON_CreateObject();
    if (json != nullptr) {
        cJSON_AddItemToObject(json, "mtu", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "isAcceptIPv4", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "isAcceptIPv6", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "isLegacy", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "isMetered", cJSON_CreateString("s"));
        cJSON_AddItemToObject(json, "isBlocking", cJSON_CreateString("s"));
        char *str = cJSON_Print(json);
        if (str != nullptr) {
            josnStr = str;
            free(str);
            str = nullptr;
            instance_->ParseJsonToConfig(vpnCfg, josnStr);
        }
        cJSON_Delete(json);
        json = nullptr;
    }
    EXPECT_EQ(vpnCfg->mtu_, 0);
}

HWTEST_F(NetworkVpnServiceExtTest, ParseJsonToConfig003, TestSize.Level1)
{
    std::string josnStr = "";
    sptr<VpnConfig> vpnCfg = new VpnConfig();
    instance_->ParseJsonToConfig(vpnCfg, josnStr);
    EXPECT_EQ(vpnCfg->mtu_, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
