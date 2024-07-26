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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "netfirewall_default_rule_parser.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class NetFirewallDefaultRuleParserTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

HWTEST_F(NetFirewallDefaultRuleParserTest, ReadJsonFile001, TestSize.Level0)
{
    std::string ret = NetFirewallDefaultRuleParser::ReadJsonFile("/system/etc/communication/netmanager_ext/1111.json");
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ConvertFirewallRuleToConfig001, TestSize.Level0)
{
    cJSON *json = cJSON_CreateObject();
    const std::string desc = "descriptionTest";
    cJSON_AddStringToObject(json, "description", desc.c_str());
    sptr<NetFirewallRule> rule = new (std::nothrow) NetFirewallRule();
    NetFirewallDefaultRuleParser::ConvertFirewallRuleToConfig(rule, json);
    EXPECT_EQ(rule->ruleDescription, desc);
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ParseIpList001, TestSize.Level0)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *localIps = cJSON_CreateArray();
    cJSON *ip = cJSON_CreateObject();
    cJSON_AddStringToObject(ip, NET_FIREWALL_IP_FAMILY.c_str(), "1");
    cJSON_AddStringToObject(ip, NET_FIREWALL_IP_TYPE.c_str(), "1");
    cJSON_AddStringToObject(ip, NET_FIREWALL_IP_MASK.c_str(), "32");
    cJSON_AddStringToObject(ip, NET_FIREWALL_IP_ADDRESS.c_str(), "192.168.4.123");
    cJSON_AddItemToArray(localIps, ip);
    cJSON_AddItemToObject(root, NET_FIREWALL_LOCAL_IP.c_str(), localIps);
    std::cout << "ParseIpList001 root=" << cJSON_Print(root) << std::endl;
    std::vector<NetFirewallIpParam> list;
    NetFirewallDefaultRuleParser::ParseIpList(list, root, NET_FIREWALL_LOCAL_IP);
    EXPECT_FALSE(list.empty());
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ParseDomainList001, TestSize.Level0)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    cJSON *param = cJSON_CreateObject();
    cJSON_AddBoolToObject(param, NET_FIREWALL_DOMAIN_IS_WILDCARD.c_str(), false);
    cJSON_AddStringToObject(param, NET_FIREWALL_DOMAIN.c_str(), "www.openharmony.cn");
    cJSON_AddItemToArray(array, param);
    cJSON_AddItemToObject(root, NET_FIREWALL_RULE_DOMAIN.c_str(), array);
    std::vector<NetFirewallDomainParam> list;
    NetFirewallDefaultRuleParser::ParseDomainList(list, root, NET_FIREWALL_RULE_DOMAIN);
    EXPECT_FALSE(list.empty());
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ParseDnsObject001, TestSize.Level0)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *param = cJSON_CreateObject();
    cJSON_AddStringToObject(param, NET_FIREWALL_DNS_PRIMARY.c_str(), "192.168.1.1");
    cJSON_AddStringToObject(param, NET_FIREWALL_DNS_STANDY.c_str(), "192.168.2.1");
    cJSON_AddItemToObject(root, NET_FIREWALL_DNS.c_str(), param);
    NetFirewallDnsParam rule;
    NetFirewallDefaultRuleParser::ParseDnsObject(rule, root, NET_FIREWALL_DNS);
    EXPECT_EQ(rule.primaryDns, "192.168.1.1");
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ConvertIpParamToConfig001, TestSize.Level0)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_FAMILY.c_str(), FAMILY_IPV6);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_TYPE.c_str(), SINGLE_IP);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_MASK.c_str(), 64);
    cJSON_AddStringToObject(root, NET_FIREWALL_IP_ADDRESS.c_str(), "AA22:BB11:1122:CDEF:1111:AA99::8888");
    NetFirewallIpParam rule;
    NetFirewallDefaultRuleParser::ConvertIpParamToConfig(rule, root);
    EXPECT_EQ(rule.type, SINGLE_IP);
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ConvertIpParamToConfig002, TestSize.Level0)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_FAMILY.c_str(), FAMILY_IPV4);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_TYPE.c_str(), SINGLE_IP);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_MASK.c_str(), 32);
    cJSON_AddStringToObject(root, NET_FIREWALL_IP_ADDRESS.c_str(), "192.168.5.4");
    NetFirewallIpParam rule;
    NetFirewallDefaultRuleParser::ConvertIpParamToConfig(rule, root);
    EXPECT_EQ(rule.type, SINGLE_IP);
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ConvertIpParamToConfig003, TestSize.Level0)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_FAMILY.c_str(), FAMILY_IPV4);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_TYPE.c_str(), MULTIPLE_IP);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_MASK.c_str(), 0);
    cJSON_AddStringToObject(root, NET_FIREWALL_IP_START.c_str(), "192.168.1.1");
    cJSON_AddStringToObject(root, NET_FIREWALL_IP_END.c_str(), "192.168.1.5");
    NetFirewallIpParam rule;
    NetFirewallDefaultRuleParser::ConvertIpParamToConfig(rule, root);
    EXPECT_EQ(rule.family, FAMILY_IPV4);
}

HWTEST_F(NetFirewallDefaultRuleParserTest, ConvertIpParamToConfig004, TestSize.Level0)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_FAMILY.c_str(), FAMILY_IPV6);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_TYPE.c_str(), MULTIPLE_IP);
    cJSON_AddNumberToObject(root, NET_FIREWALL_IP_MASK.c_str(), 1);
    cJSON_AddStringToObject(root, NET_FIREWALL_IP_START.c_str(), "AA22:BB11:1122:CDEF:1111:AA99::8888");
    cJSON_AddStringToObject(root, NET_FIREWALL_IP_END.c_str(), "AA22:BB11:1122:CDEF:1111:AA99::9999");
    NetFirewallIpParam rule;
    NetFirewallDefaultRuleParser::ConvertIpParamToConfig(rule, root);
    EXPECT_EQ(rule.family, FAMILY_IPV6);
}
}
}