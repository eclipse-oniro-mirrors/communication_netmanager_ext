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

#include "vpn_data_bean.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class VpnDataBeanTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void VpnDataBeanTest::SetUpTestCase() {}

void VpnDataBeanTest::TearDownTestCase() {}

void VpnDataBeanTest::SetUp() {}

void VpnDataBeanTest::TearDown() {}

HWTEST_F(VpnDataBeanTest, ConvertVpnBeanToSysVpnConfig001, TestSize.Level1)
{
    sptr<VpnDataBean> bean = nullptr;
    EXPECT_EQ(VpnDataBean::ConvertVpnBeanToSysVpnConfig(bean), nullptr);
    bean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(bean, nullptr);
    bean->vpnType_ = -1;
    EXPECT_EQ(VpnDataBean::ConvertVpnBeanToSysVpnConfig(bean), nullptr);
    bean->vpnType_ = VpnType::L2TP_IPSEC_RSA;
    EXPECT_NE(VpnDataBean::ConvertVpnBeanToSysVpnConfig(bean), nullptr);
}

HWTEST_F(VpnDataBeanTest, ConvertVpnBeanToIpsecVpnConfig001, TestSize.Level1)
{
    sptr<VpnDataBean> bean = nullptr;
    EXPECT_EQ(VpnDataBean::ConvertVpnBeanToIpsecVpnConfig(bean), nullptr);
}

HWTEST_F(VpnDataBeanTest, ConvertVpnBeanToL2tpVpnConfig001, TestSize.Level1)
{
    sptr<VpnDataBean> bean = nullptr;
    EXPECT_EQ(VpnDataBean::ConvertVpnBeanToL2tpVpnConfig(bean), nullptr);
}

HWTEST_F(VpnDataBeanTest, ConvertSysVpnConfigToVpnBean001, TestSize.Level1)
{
    sptr<SysVpnConfig> config = nullptr;
    EXPECT_EQ(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config = new (std::nothrow) IpsecVpnConfig();
    config->vpnType_ = VpnType::IKEV2_IPSEC_MSCHAPv2;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config->vpnType_ = VpnType::IKEV2_IPSEC_PSK;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config->vpnType_ = VpnType::IKEV2_IPSEC_RSA;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config->vpnType_ = VpnType::IPSEC_XAUTH_PSK;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config->vpnType_ = VpnType::IPSEC_XAUTH_RSA;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config->vpnType_ = VpnType::IPSEC_HYBRID_RSA;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config = nullptr;
    config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnType_ = VpnType::L2TP_IPSEC_PSK;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config->vpnType_ = VpnType::L2TP_IPSEC_RSA;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
    config->vpnType_ = -1;
    EXPECT_NE(VpnDataBean::ConvertSysVpnConfigToVpnBean(config), nullptr);
}

HWTEST_F(VpnDataBeanTest, ConvertCommonVpnConfigToVpnBean001, TestSize.Level1)
{
    sptr<VpnDataBean> bean = nullptr;
    sptr<SysVpnConfig> config = nullptr;
    VpnDataBean::ConvertCommonVpnConfigToVpnBean(config, bean);
    bean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(bean, nullptr);
    std::string testId = "testConvertCommonVpnConfigToVpnBean";
    bean->vpnId_ = testId;
    VpnDataBean::ConvertCommonVpnConfigToVpnBean(config, bean);
    EXPECT_EQ(bean->vpnId_, testId);
    config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "testConvertCommonVpnConfigToVpnBean1";
    config->vpnType_ = VpnType::L2TP_IPSEC_PSK;
    INetAddr ipv4Addr;
    ipv4Addr.type_ = INetAddr::IPV4;
    ipv4Addr.family_ = 0x01;
    ipv4Addr.prefixlen_ = 0x01;
    ipv4Addr.address_ = "172.17.5.234";
    ipv4Addr.netMask_ = "255.255.254.0";
    ipv4Addr.hostName_ = "netAddr";
    config->addresses_.push_back(ipv4Addr);
    std::string dnsServer = "192.168.2.0";
    config->dnsAddresses_.push_back(dnsServer);
    std::string domain = "openharmony.cn";
    config->searchDomains_.push_back(domain);
    VpnDataBean::ConvertCommonVpnConfigToVpnBean(config, bean);
    EXPECT_EQ(bean->vpnId_, config->vpnId_);
}

HWTEST_F(VpnDataBeanTest, ConvertIpsecVpnConfigToVpnBean001, TestSize.Level1)
{
    sptr<VpnDataBean> bean = nullptr;
    sptr<IpsecVpnConfig> config = nullptr;
    VpnDataBean::ConvertIpsecVpnConfigToVpnBean(config, bean);
    bean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(bean, nullptr);
    std::string testId = "ConvertIpsecVpnConfigToVpnBean";
    bean->vpnId_ = testId;
    VpnDataBean::ConvertIpsecVpnConfigToVpnBean(config, bean);
    EXPECT_EQ(bean->vpnId_, testId);
}

HWTEST_F(VpnDataBeanTest, ConvertL2tpVpnConfigToVpnBean001, TestSize.Level1)
{
    sptr<VpnDataBean> bean = nullptr;
    sptr<L2tpVpnConfig> config = nullptr;
    VpnDataBean::ConvertL2tpVpnConfigToVpnBean(config, bean);
    bean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(bean, nullptr);
    std::string testId = "ConvertL2tpVpnConfigToVpnBean";
    bean->vpnId_ = testId;
    VpnDataBean::ConvertL2tpVpnConfigToVpnBean(config, bean);
    EXPECT_EQ(bean->vpnId_, testId);

    config = new (std::nothrow) L2tpVpnConfig();
    ASSERT_NE(config, nullptr);
    config->vpnId_ = "ConvertL2tpVpnConfigToVpnBean1";
    config->vpnType_ = VpnType::L2TP_IPSEC_PSK;
    config->ipsecPreSharedKey_ = "ipsecPreSharedKeyTest";
    VpnDataBean::ConvertL2tpVpnConfigToVpnBean(config, bean);
    EXPECT_EQ(bean->ipsecPreSharedKey_, config->ipsecPreSharedKey_);
}
} // namespace NetManagerStandard
} // namespace OHOS