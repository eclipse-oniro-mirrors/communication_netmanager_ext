/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <arpa/inet.h>

#include "napi_utils.h"
#include "netfirewall_common.h"
#include "net_manager_constants.h"
#include <regex>

#include "net_firewall_rule_parse.h"
#include "net_firewall_param_check.h"
#include "mock_napi.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class NetFirewallPolicyTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class RequestParamTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class FirewallRulePageTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class InterceptRecordPageTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class TrafficFilterIPTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class TrafficFilterPortTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class TrafficFilterRedirectRuleTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

class InterfaceNameValidationTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(NetFirewallPolicyTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<NetFirewallPolicy> ptr = NetFirewallPolicy::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(NetFirewallPolicyTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    bool isOpen = true;
    parcel.WriteBool(isOpen);
    sptr<NetFirewallPolicy> ptr = NetFirewallPolicy::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(NetFirewallPolicyTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    bool isOpen = true;
    parcel.WriteBool(isOpen);
    int32_t inAction = 1;
    parcel.WriteInt32(inAction);
    sptr<NetFirewallPolicy> ptr = NetFirewallPolicy::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(RequestParamTest, Unmarshalling004, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t orderField = 1;
    parcel.WriteInt32(orderField);
    sptr<RequestParam> ptr = RequestParam::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling004, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(FirewallRulePageTest, Unmarshalling005, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    uint32_t size = 2;
    parcel.WriteUint32(size);
    sptr<FirewallRulePage> ptr = FirewallRulePage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling004, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(InterceptRecordPageTest, Unmarshalling005, TestSize.Level0)
{
    Parcel parcel;
    int32_t page = 1;
    parcel.WriteInt32(page);
    int32_t pageSize = 2;
    parcel.WriteInt32(pageSize);
    int32_t totalPage = 50;
    parcel.WriteInt32(totalPage);
    uint32_t size = 2;
    parcel.WriteUint32(size);
    sptr<InterceptRecordPage> ptr = InterceptRecordPage::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

// TrafficFilterIPAddress tests
HWTEST_F(TrafficFilterIPTest, IPAddressUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterIPAddress> ptr = TrafficFilterIPAddress::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPAddressUnmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteInt32(1);
    for (int i = 0; i < 15; i++) {
        parcel.WriteUint8(0);
    }
    sptr<TrafficFilterIPAddress> ptr = TrafficFilterIPAddress::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPAddressMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterIPAddress ipAddr;
    ipAddr.family_ = 1; // IPv4
    inet_pton(AF_INET, "192.168.1.1", ipAddr.addr_);

    Parcel parcel;
    EXPECT_TRUE(ipAddr.Marshalling(parcel));

    sptr<TrafficFilterIPAddress> result = TrafficFilterIPAddress::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->family_, 1);
}

HWTEST_F(TrafficFilterIPTest, IPAddressMarshallingAndUnmarshalling002, TestSize.Level0)
{
    TrafficFilterIPAddress ipAddr;
    ipAddr.family_ = 2; // IPv6
    inet_pton(AF_INET6, "fe80::1", ipAddr.addr_);

    Parcel parcel;
    EXPECT_TRUE(ipAddr.Marshalling(parcel));

    sptr<TrafficFilterIPAddress> result = TrafficFilterIPAddress::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->family_, 2);
}

// TrafficFilterIPCidr tests
HWTEST_F(TrafficFilterIPTest, IPCidrUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterIPCidr> ptr = TrafficFilterIPCidr::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPCidrMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterIPCidr cidr;
    cidr.base_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.0", cidr.base_.addr_);
    cidr.prefixLen_ = 24;

    Parcel parcel;
    EXPECT_TRUE(cidr.Marshalling(parcel));

    sptr<TrafficFilterIPCidr> result = TrafficFilterIPCidr::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->base_.family_, 1);
    EXPECT_EQ(result->prefixLen_, 24);
}

// TrafficFilterIPRange tests
HWTEST_F(TrafficFilterIPTest, IPRangeUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterIPRange> ptr = TrafficFilterIPRange::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPRangeMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterIPRange range;
    range.start_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.1", range.start_.addr_);
    range.end_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.100", range.end_.addr_);

    Parcel parcel;
    EXPECT_TRUE(range.Marshalling(parcel));

    sptr<TrafficFilterIPRange> result = TrafficFilterIPRange::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->start_.family_, 1);
    EXPECT_EQ(result->end_.family_, 1);
}

// TrafficFilterIPMulti tests
HWTEST_F(TrafficFilterIPTest, IPMultiUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterIPMulti> ptr = TrafficFilterIPMulti::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPMultiUnmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteUint32(2);
    parcel.WriteInt32(1);
    for (int i = 0; i < 16; i++) {
        parcel.WriteUint8(0);
    }
    sptr<TrafficFilterIPMulti> ptr = TrafficFilterIPMulti::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPMultiUnmarshalling003, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteUint32(17); // Exceeds MAX_MULTI_IP_COUNT
    sptr<TrafficFilterIPMulti> ptr = TrafficFilterIPMulti::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPMultiMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterIPMulti multi;
    multi.ipCount_ = 2;
    multi.ips_[0].family_ = 1;
    inet_pton(AF_INET, "192.168.1.1", multi.ips_[0].addr_);
    multi.ips_[1].family_ = 1;
    inet_pton(AF_INET, "192.168.1.2", multi.ips_[1].addr_);

    Parcel parcel;
    EXPECT_TRUE(multi.Marshalling(parcel));

    sptr<TrafficFilterIPMulti> result = TrafficFilterIPMulti::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->ipCount_, 2);
}

// TrafficFilterIPMatch tests
HWTEST_F(TrafficFilterIPTest, IPMatchUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterIPMatch> ptr = TrafficFilterIPMatch::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPMatchUnmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteInt32(5); // Invalid type
    sptr<TrafficFilterIPMatch> ptr = TrafficFilterIPMatch::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, IPMatchMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = 0; // ANY
    ipMatch.invert_ = false;

    Parcel parcel;
    EXPECT_TRUE(ipMatch.Marshalling(parcel));

    sptr<TrafficFilterIPMatch> result = TrafficFilterIPMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 0);
    EXPECT_EQ(result->invert_, false);
}

HWTEST_F(TrafficFilterIPTest, IPMatchMarshallingAndUnmarshalling002, TestSize.Level0)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = 1; // SINGLE
    ipMatch.invert_ = false;
    ipMatch.single_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.1", ipMatch.single_.addr_);

    Parcel parcel;
    EXPECT_TRUE(ipMatch.Marshalling(parcel));

    sptr<TrafficFilterIPMatch> result = TrafficFilterIPMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 1);
    EXPECT_EQ(result->invert_, false);
}

HWTEST_F(TrafficFilterIPTest, IPMatchMarshallingAndUnmarshalling003, TestSize.Level0)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = 2; // CIDR
    ipMatch.invert_ = false;
    ipMatch.cidr_.base_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.0", ipMatch.cidr_.base_.addr_);
    ipMatch.cidr_.prefixLen_ = 24;

    Parcel parcel;
    EXPECT_TRUE(ipMatch.Marshalling(parcel));

    sptr<TrafficFilterIPMatch> result = TrafficFilterIPMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 2);
    EXPECT_EQ(result->cidr_.prefixLen_, 24);
}

HWTEST_F(TrafficFilterIPTest, IPMatchMarshallingAndUnmarshalling004, TestSize.Level0)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = 3; // RANGE
    ipMatch.invert_ = false;
    ipMatch.range_.start_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.1", ipMatch.range_.start_.addr_);
    ipMatch.range_.end_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.100", ipMatch.range_.end_.addr_);

    Parcel parcel;
    EXPECT_TRUE(ipMatch.Marshalling(parcel));

    sptr<TrafficFilterIPMatch> result = TrafficFilterIPMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 3);
}

HWTEST_F(TrafficFilterIPTest, IPMatchMarshallingAndUnmarshalling005, TestSize.Level0)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = 4; // MULTI
    ipMatch.invert_ = false;
    ipMatch.multi_.ipCount_ = 1;
    ipMatch.multi_.ips_[0].family_ = 1;
    inet_pton(AF_INET, "192.168.1.1", ipMatch.multi_.ips_[0].addr_);

    Parcel parcel;
    EXPECT_TRUE(ipMatch.Marshalling(parcel));

    sptr<TrafficFilterIPMatch> result = TrafficFilterIPMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 4);
}

HWTEST_F(TrafficFilterIPTest, IPMatchMarshallingAndUnmarshalling006, TestSize.Level0)
{
    TrafficFilterIPMatch ipMatch;
    ipMatch.type_ = 1;
    ipMatch.invert_ = true;
    ipMatch.single_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.1", ipMatch.single_.addr_);

    Parcel parcel;
    EXPECT_TRUE(ipMatch.Marshalling(parcel));

    sptr<TrafficFilterIPMatch> result = TrafficFilterIPMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 1);
    EXPECT_EQ(result->invert_, true);
}

// TrafficFilterPortRange tests
HWTEST_F(TrafficFilterPortTest, PortRangeUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterPortRange> ptr = TrafficFilterPortRange::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterPortTest, PortRangeUnmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteUint16(80);
    sptr<TrafficFilterPortRange> ptr = TrafficFilterPortRange::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterPortTest, PortRangeMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterPortRange portRange;
    portRange.startPort_ = 80;
    portRange.endPort_ = 443;

    Parcel parcel;
    EXPECT_TRUE(portRange.Marshalling(parcel));

    sptr<TrafficFilterPortRange> result = TrafficFilterPortRange::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->startPort_, 80);
    EXPECT_EQ(result->endPort_, 443);
}

// TrafficFilterPortMulti tests
HWTEST_F(TrafficFilterPortTest, PortMultiUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterPortMulti> ptr = TrafficFilterPortMulti::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterPortTest, PortMultiUnmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteUint32(65); // Exceeds MAX_MULTI_PORT_COUNT
    sptr<TrafficFilterPortMulti> ptr = TrafficFilterPortMulti::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterPortTest, PortMultiMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterPortMulti portMulti;
    portMulti.portCount_ = 3;
    portMulti.ports_[0] = 80;
    portMulti.ports_[1] = 443;
    portMulti.ports_[2] = 8080;

    Parcel parcel;
    EXPECT_TRUE(portMulti.Marshalling(parcel));

    sptr<TrafficFilterPortMulti> result = TrafficFilterPortMulti::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->portCount_, 3);
    EXPECT_EQ(result->ports_[0], 80);
    EXPECT_EQ(result->ports_[1], 443);
    EXPECT_EQ(result->ports_[2], 8080);
}

// TrafficFilterPortMatch tests
HWTEST_F(TrafficFilterPortTest, PortMatchUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterPortMatch> ptr = TrafficFilterPortMatch::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterPortTest, PortMatchUnmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteInt32(4); // Invalid type
    sptr<TrafficFilterPortMatch> ptr = TrafficFilterPortMatch::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterPortTest, PortMatchMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterPortMatch portMatch;
    portMatch.type_ = 0; // ANY
    portMatch.invert_ = false;

    Parcel parcel;
    EXPECT_TRUE(portMatch.Marshalling(parcel));

    sptr<TrafficFilterPortMatch> result = TrafficFilterPortMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 0);
    EXPECT_EQ(result->invert_, false);
}

HWTEST_F(TrafficFilterPortTest, PortMatchMarshallingAndUnmarshalling002, TestSize.Level0)
{
    TrafficFilterPortMatch portMatch;
    portMatch.type_ = 1; // SINGLE
    portMatch.invert_ = false;
    portMatch.single_ = 443;

    Parcel parcel;
    EXPECT_TRUE(portMatch.Marshalling(parcel));

    sptr<TrafficFilterPortMatch> result = TrafficFilterPortMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 1);
    EXPECT_EQ(result->single_, 443);
}

HWTEST_F(TrafficFilterPortTest, PortMatchMarshallingAndUnmarshalling003, TestSize.Level0)
{
    TrafficFilterPortMatch portMatch;
    portMatch.type_ = 2; // RANGE
    portMatch.invert_ = false;
    portMatch.range_.startPort_ = 80;
    portMatch.range_.endPort_ = 443;

    Parcel parcel;
    EXPECT_TRUE(portMatch.Marshalling(parcel));

    sptr<TrafficFilterPortMatch> result = TrafficFilterPortMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 2);
    EXPECT_EQ(result->range_.startPort_, 80);
    EXPECT_EQ(result->range_.endPort_, 443);
}

HWTEST_F(TrafficFilterPortTest, PortMatchMarshallingAndUnmarshalling004, TestSize.Level0)
{
    TrafficFilterPortMatch portMatch;
    portMatch.type_ = 3; // MULTI
    portMatch.invert_ = false;
    portMatch.multi_.portCount_ = 2;
    portMatch.multi_.ports_[0] = 80;
    portMatch.multi_.ports_[1] = 443;

    Parcel parcel;
    EXPECT_TRUE(portMatch.Marshalling(parcel));

    sptr<TrafficFilterPortMatch> result = TrafficFilterPortMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type_, 3);
}

// TrafficFilterInterfaceMatch tests
HWTEST_F(TrafficFilterIPTest, InterfaceMatchUnmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterInterfaceMatch> ptr = TrafficFilterInterfaceMatch::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, InterfaceMatchUnmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteBool(true);
    sptr<TrafficFilterInterfaceMatch> ptr = TrafficFilterInterfaceMatch::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterIPTest, InterfaceMatchMarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterInterfaceMatch interfaceMatch;
    interfaceMatch.enabled_ = true;
    interfaceMatch.invert_ = false;
    interfaceMatch.isPrefix_ = false;
    interfaceMatch.ifName_ = "eth0";

    Parcel parcel;
    EXPECT_TRUE(interfaceMatch.Marshalling(parcel));

    sptr<TrafficFilterInterfaceMatch> result = TrafficFilterInterfaceMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->enabled_, true);
    EXPECT_EQ(result->invert_, false);
    EXPECT_EQ(result->isPrefix_, false);
    EXPECT_EQ(result->ifName_, "eth0");
}

HWTEST_F(TrafficFilterIPTest, InterfaceMatchMarshallingAndUnmarshalling002, TestSize.Level0)
{
    TrafficFilterInterfaceMatch interfaceMatch;
    interfaceMatch.enabled_ = false;
    interfaceMatch.invert_ = true;
    interfaceMatch.isPrefix_ = true;
    interfaceMatch.ifName_ = "wlan";

    Parcel parcel;
    EXPECT_TRUE(interfaceMatch.Marshalling(parcel));

    sptr<TrafficFilterInterfaceMatch> result = TrafficFilterInterfaceMatch::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->enabled_, false);
    EXPECT_EQ(result->invert_, true);
    EXPECT_EQ(result->isPrefix_, true);
    EXPECT_EQ(result->ifName_, "wlan");
}

// TrafficFilterRedirectRule tests
HWTEST_F(TrafficFilterRedirectRuleTest, Unmarshalling001, TestSize.Level0)
{
    Parcel parcel;
    sptr<TrafficFilterRedirectRule> ptr = TrafficFilterRedirectRule::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterRedirectRuleTest, Unmarshalling002, TestSize.Level0)
{
    Parcel parcel;
    parcel.WriteUint32(100);
    sptr<TrafficFilterRedirectRule> ptr = TrafficFilterRedirectRule::Unmarshalling(parcel);
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F(TrafficFilterRedirectRuleTest, MarshallingAndUnmarshalling001, TestSize.Level0)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 100;
    rule.hookPoint_ = 3; // PREROUTING
    rule.protocol_ = 6; // TCP
    rule.srcIp_.type_ = 0; // ANY
    rule.srcPort_.type_ = 0; // ANY
    rule.dstIp_.type_ = 1; // SINGLE
    rule.dstIp_.single_.family_ = 1;
    inet_pton(AF_INET, "93.184.216.34", rule.dstIp_.single_.addr_);
    rule.dstPort_.type_ = 1; // SINGLE
    rule.dstPort_.single_ = 443;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = false;
    rule.uidStart_ = 10042;
    rule.uidEnd_ = 10042;
    rule.proxyIp_.family_ = 1;
    inet_pton(AF_INET, "127.0.0.1", rule.proxyIp_.addr_);
    rule.proxyPort_ = 8080;

    Parcel parcel;
    EXPECT_TRUE(rule.Marshalling(parcel));

    sptr<TrafficFilterRedirectRule> result = TrafficFilterRedirectRule::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->priority_, 100);
    EXPECT_EQ(result->hookPoint_, 3);
    EXPECT_EQ(result->protocol_, 6);
    EXPECT_EQ(result->dstIp_.type_, 1);
    EXPECT_EQ(result->dstPort_.type_, 1);
    EXPECT_EQ(result->dstPort_.single_, 443);
    EXPECT_EQ(result->uidStart_, 10042);
    EXPECT_EQ(result->uidEnd_, 10042);
    EXPECT_EQ(result->proxyPort_, 8080);
}

HWTEST_F(TrafficFilterRedirectRuleTest, MarshallingAndUnmarshalling002, TestSize.Level0)
{
    TrafficFilterRedirectRule rule;
    rule.priority_ = 200;
    rule.hookPoint_ = 1; // OUTPUT
    rule.protocol_ = 17; // UDP
    rule.srcIp_.type_ = 1; // SINGLE
    rule.srcIp_.single_.family_ = 1;
    inet_pton(AF_INET, "192.168.1.100", rule.srcIp_.single_.addr_);
    rule.srcPort_.type_ = 1; // SINGLE
    rule.srcPort_.single_ = 50000;
    rule.dstIp_.type_ = 1; // SINGLE
    rule.dstIp_.single_.family_ = 1;
    inet_pton(AF_INET, "8.8.8.8", rule.dstIp_.single_.addr_);
    rule.dstPort_.type_ = 1; // SINGLE
    rule.dstPort_.single_ = 53;
    rule.inInterface_.enabled_ = false;
    rule.outInterface_.enabled_ = true;
    rule.outInterface_.isPrefix_ = true;
    rule.outInterface_.ifName_ = "eth";
    rule.uidStart_ = 0;
    rule.uidEnd_ = 0;
    rule.proxyIp_.family_ = 1;
    inet_pton(AF_INET, "127.0.0.1", rule.proxyIp_.addr_);
    rule.proxyPort_ = 5353;

    Parcel parcel;
    EXPECT_TRUE(rule.Marshalling(parcel));

    sptr<TrafficFilterRedirectRule> result = TrafficFilterRedirectRule::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->priority_, 200);
    EXPECT_EQ(result->hookPoint_, 1);
    EXPECT_EQ(result->protocol_, 17);
    EXPECT_EQ(result->srcIp_.type_, 1);
    EXPECT_EQ(result->srcPort_.type_, 1);
    EXPECT_EQ(result->srcPort_.single_, 50000);
    EXPECT_EQ(result->dstPort_.type_, 1);
    EXPECT_EQ(result->dstPort_.single_, 53);
    EXPECT_EQ(result->outInterface_.enabled_, true);
    EXPECT_EQ(result->outInterface_.isPrefix_, true);
    EXPECT_EQ(result->outInterface_.ifName_, "eth");
    EXPECT_EQ(result->proxyPort_, 5353);
}

HWTEST_F(InterfaceNameValidationTest, CheckRuleStringParamInterface001, TestSize.Level0)
{
    SetMockHasStringProperties(false);
    SetMockHasInterface(true);
    napi_env env = nullptr;
    napi_value object = nullptr;
    int32_t ret = NetFirewallParamCheck::CheckRuleStringParam(env, object);
    EXPECT_EQ(ret, FIREWALL_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceNameValidationTest, CheckRuleStringParamInterface002, TestSize.Level0)
{
    SetMockHasStringProperties(false);
    SetMockHasInterface(false);
    napi_env env = nullptr;
    napi_value object = nullptr;
    int32_t ret = NetFirewallParamCheck::CheckRuleStringParam(env, object);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName001, TestSize.Level0)
{
    std::regex pattern {"^[a-zA-Z][a-zA-Z0-9_-]{0,14}$"};
    EXPECT_TRUE(std::regex_match("wlan0", pattern));
    EXPECT_TRUE(std::regex_match("eth0", pattern));
    EXPECT_TRUE(std::regex_match("lo", pattern));
    EXPECT_TRUE(std::regex_match("en0", pattern));
    EXPECT_TRUE(std::regex_match("usb0", pattern));
    EXPECT_TRUE(std::regex_match("br-lan", pattern));
    EXPECT_TRUE(std::regex_match("docker0", pattern));
    EXPECT_TRUE(std::regex_match("eth0_1", pattern));
    EXPECT_TRUE(std::regex_match("wg0-server", pattern));
}
 
HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName002, TestSize.Level0)
{
    std::regex pattern {"^[a-zA-Z][a-zA-Z0-9_-]{0,14}$"};
    EXPECT_FALSE(std::regex_match("", pattern));
    EXPECT_FALSE(std::regex_match("0wlan", pattern));
    EXPECT_FALSE(std::regex_match("_eth0", pattern));
    EXPECT_FALSE(std::regex_match("-br0", pattern));
    EXPECT_FALSE(std::regex_match(".lo", pattern));
    EXPECT_FALSE(std::regex_match("wlan@0", pattern));
    EXPECT_FALSE(std::regex_match("eth 0", pattern));
    EXPECT_FALSE(std::regex_match("wlan0.100", pattern));
    EXPECT_FALSE(std::regex_match("a123456789012345", pattern));
}
 
HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName003, TestSize.Level0)
{
    std::regex pattern {"^[a-zA-Z][a-zA-Z0-9_-]{0,14}$"};
    EXPECT_TRUE(std::regex_match("WLAN0", pattern));
    EXPECT_TRUE(std::regex_match("Eth0", pattern));
    EXPECT_TRUE(std::regex_match("a", pattern));
    EXPECT_TRUE(std::regex_match("Z9", pattern));
    EXPECT_TRUE(std::regex_match("A", pattern));
    EXPECT_TRUE(std::regex_match("a12345678901234", pattern));
    EXPECT_FALSE(std::regex_match("a123456789012345", pattern));
    EXPECT_FALSE(std::regex_match("a_b-c0d_e1f2g3h4i", pattern));
}

HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName004, TestSize.Level0)
{
    EXPECT_EQ(MAX_INTERFACE_NAME_LEN, 16);
    EXPECT_EQ(MAX_RULE_INTERFACE_COUNT, 10);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName005, TestSize.Level0)
{
    std::regex pattern {"^[a-zA-Z][a-zA-Z0-9_-]{0,14}$"};
    // valid names with underscore
    EXPECT_TRUE(std::regex_match("a_1", pattern));
    EXPECT_TRUE(std::regex_match("test_iface", pattern));
    EXPECT_TRUE(std::regex_match("my_network_0", pattern));
    // valid names with hyphen
    EXPECT_TRUE(std::regex_match("br-lan0", pattern));
    EXPECT_TRUE(std::regex_match("docker-br", pattern));
    // mixed special chars
    EXPECT_TRUE(std::regex_match("a-b_c", pattern));
    EXPECT_TRUE(std::regex_match("x_y-z", pattern));
    // invalid special chars
    EXPECT_FALSE(std::regex_match("eth0!", pattern));
    EXPECT_FALSE(std::regex_match("eth0#", pattern));
    EXPECT_FALSE(std::regex_match("eth0%", pattern));
    EXPECT_FALSE(std::regex_match("eth0^", pattern));
    EXPECT_FALSE(std::regex_match("eth0*", pattern));
    EXPECT_FALSE(std::regex_match("eth0+", pattern));
    EXPECT_FALSE(std::regex_match("eth0=", pattern));
    EXPECT_FALSE(std::regex_match("eth0/", pattern));
    EXPECT_FALSE(std::regex_match("eth0\\", pattern));
}

HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName006, TestSize.Level0)
{
    int32_t ret = NetFirewallParamCheck::CheckInterfaceName("");
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);

    ret = NetFirewallParamCheck::CheckInterfaceName("invalid_with_special_@");
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);

    ret = NetFirewallParamCheck::CheckInterfaceName("0eth0");
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);

    ret = NetFirewallParamCheck::CheckInterfaceName("eth0");
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName007, TestSize.Level0)
{
    int32_t ret = NetFirewallParamCheck::CheckInterfaceName("a1234567890123456");
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);

    ret = NetFirewallParamCheck::CheckInterfaceName("a12345678901234");
    EXPECT_EQ(ret, FIREWALL_SUCCESS);

    ret = NetFirewallParamCheck::CheckInterfaceName("a123456789012345");
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);

    ret = NetFirewallParamCheck::CheckInterfaceName("a");
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterfaceName008, TestSize.Level0)
{
    int32_t ret = NetFirewallParamCheck::CheckInterfaceName("eth_0");
    EXPECT_EQ(ret, FIREWALL_SUCCESS);

    ret = NetFirewallParamCheck::CheckInterfaceName("br-lan");
    EXPECT_EQ(ret, FIREWALL_SUCCESS);

    ret = NetFirewallParamCheck::CheckInterfaceName("eth0!");
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);

    ret = NetFirewallParamCheck::CheckInterfaceName("eth0.");
    EXPECT_EQ(ret, FIREWALL_ERR_INVALID_PARAMETER);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterface001, TestSize.Level0)
{
    SetMockHasStringProperties(false);
    SetMockHasInterface(true);
    SetMockValueType(napi_undefined);
    napi_env env = nullptr;
    napi_value object = nullptr;
    int32_t ret = NetFirewallParamCheck::CheckInterface(env, object);
    EXPECT_EQ(ret, FIREWALL_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterface002, TestSize.Level0)
{
    SetMockHasStringProperties(false);
    SetMockHasInterface(true);
    SetMockValueType(napi_string);
    SetMockStringResult("");
    napi_env env = nullptr;
    napi_value object = nullptr;
    int32_t ret = NetFirewallParamCheck::CheckInterface(env, object);
    EXPECT_EQ(ret, FIREWALL_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterface003, TestSize.Level0)
{
    SetMockHasStringProperties(false);
    SetMockHasInterface(true);
    SetMockValueType(napi_string);
    SetMockStringResult("eth0");
    napi_env env = nullptr;
    napi_value object = nullptr;
    int32_t ret = NetFirewallParamCheck::CheckInterface(env, object);
    EXPECT_EQ(ret, FIREWALL_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceNameValidationTest, CheckInterface004, TestSize.Level0)
{
    SetMockHasStringProperties(false);
    SetMockHasInterface(true);
    SetMockValueType(napi_string);
    SetMockStringResult("invalid!");
    napi_env env = nullptr;
    napi_value object = nullptr;
    int32_t ret = NetFirewallParamCheck::CheckInterface(env, object);
    EXPECT_EQ(ret, FIREWALL_ERR_PARAMETER_ERROR);
}

class NetFirewallRuleParseTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(NetFirewallRuleParseTest, ParseInterface001, TestSize.Level0)
{
    napi_env env = nullptr;
    napi_value value = nullptr;
    std::string interfaceName = "initial";
    NetFirewallRuleParse::ParseInterface(env, value, interfaceName);
    EXPECT_EQ(interfaceName, "initial");
}

HWTEST_F(NetFirewallRuleParseTest, ParseInterface002, TestSize.Level0)
{
    napi_env env = nullptr;
    napi_value value = nullptr;
    std::string interfaceName;
    NetFirewallRuleParse::ParseInterface(env, value, interfaceName);
    EXPECT_TRUE(interfaceName.empty());
}
}
}