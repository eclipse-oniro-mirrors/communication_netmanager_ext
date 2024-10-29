/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *TEST_TEXT = "adfsjfjkfk^#$ajf!@!#$#kjd nck?fgnf<kdjnf>kjask?.fcnvdkjfn kjdkj.,.vd";
constexpr const char *SPLIT = "?";
constexpr const char *TEST_IP = "155.153.144.154";
constexpr const char *TEST_IPV4 = "534/6::45/144.15:4::44";
constexpr const char *TEST_DOMAIN1 = "123445";
constexpr const char *TEST_DOMAIN2 = ".com";
constexpr const char *TEST_DOMAIN3 = "test.com";
constexpr const char *TEST_DOMAIN4 = "testcom";
constexpr const char *TEST_DOMAIN5 = "com.test";
constexpr const char *TEST_DOMAIN6 = "test.co.uk";
constexpr const char *TEST_DOMAIN7 = "test.com.com";
constexpr const char *TEST_DOMAIN8 = "test1.test2.test3.test4.test5.com";
constexpr const char *DEFAULT_IPV6_ANY_INIT_ADDR = "::";

const std::string TEST_DOMAIN9 = "www.test.com";
const std::string TEST_DOMAIN10 = "*";
const std::string TEST_DOMAIN11 = "";
const std::string TEST_DOMAIN12 = "*.test.*";
const std::string TEST_DOMAIN13 = "*.test./{*";

constexpr int32_t MAX_IPV6_PREFIX_LENGTH = 128;
constexpr uint32_t ADDREDD_LEN = 16;
constexpr int32_t BIT_32 = 32;
constexpr int32_t BIT_24 = 24;
constexpr int32_t BIT_16 = 16;
constexpr int32_t BIT_8 = 8;
} // namespace
class UtNetmanagerBaseCommon : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void UtNetmanagerBaseCommon::SetUpTestCase() {}

void UtNetmanagerBaseCommon::TearDownTestCase() {}

void UtNetmanagerBaseCommon::SetUp() {}

void UtNetmanagerBaseCommon::TearDown() {}

/**
 * @tc.name: UtNetmanagerBaseCommon001
 * @tc.desc: Test UtNetmanagerBaseCommon ForkExec.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, UtNetmanagerBaseCommon001, TestSize.Level1)
{
    std::string out;
    CommonUtils::ForkExec("/system/bin/ls -a", &out);
    ASSERT_FALSE(out.empty());
    std::cout << "out: " << out << std::endl;
}

/**
 * @tc.name: UtNetmanagerBaseCommon002
 * @tc.desc: Test UtNetmanagerBaseCommon ForkExec.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, UtNetmanagerBaseCommon002, TestSize.Level1)
{
    std::string out;
    CommonUtils::ForkExec("/system/bin/ls -l", &out);
    ASSERT_FALSE(out.empty());
    std::cout << "out: " << out << std::endl;
}

/**
 * @tc.name: UtNetmanagerBaseCommon003
 * @tc.desc: Test UtNetmanagerBaseCommon ForkExec.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, UtNetmanagerBaseCommon003, TestSize.Level1)
{
    CommonUtils::ForkExec("/system/bin/mount -o rw,remount /");
    CommonUtils::ForkExec("/system/bin/mkdir uttest");
    std::string out;
    CommonUtils::ForkExec("/system/bin/ls -a", &out);
    ASSERT_TRUE(out.find("uttest") != std::string::npos);
    CommonUtils::ForkExec("/system/bin/rm -rf uttest");
}

/**
 * @tc.name: SplitTest001
 * @tc.desc: Test UtNetmanagerBaseCommon Split.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, SplitTest001, TestSize.Level1)
{
    std::vector<std::string> result = CommonUtils::Split(TEST_TEXT, SPLIT);
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: SplitTest002
 * @tc.desc: Test UtNetmanagerBaseCommon Split.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, SplitTest002, TestSize.Level1)
{
    std::vector<std::string> result = CommonUtils::Split({}, SPLIT);
    ASSERT_TRUE(result.empty());
}

/**
 * @tc.name: StripTest001
 * @tc.desc: Test UtNetmanagerBaseCommon Strip.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StripTest001, TestSize.Level1)
{
    auto result = CommonUtils::Strip(TEST_TEXT, '?');
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: IsValidIPV4Test001
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV4.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV4Test001, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV4(TEST_TEXT);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidIPV4Test002
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV4.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV4Test002, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV4({});
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidIPV6Test001
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV6.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV6Test001, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV6(TEST_TEXT);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidIPV6Test002
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV6.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV6Test002, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV6({});
    ASSERT_FALSE(result);
}

/**
 * @tc.name: GetAddrFamilyTest001
 * @tc.desc: Test UtNetmanagerBaseCommon GetAddrFamily.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, GetAddrFamilyTest001, TestSize.Level1)
{
    auto result = CommonUtils::GetAddrFamily(TEST_IP);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: GetMaskLengthTest001
 * @tc.desc: Test UtNetmanagerBaseCommon GetMaskLength.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, GetMaskLengthTest001, TestSize.Level1)
{
    auto result = CommonUtils::GetMaskLength(TEST_TEXT);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: ConvertIpv4AddressTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest001, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(0);
    ASSERT_TRUE(result.empty());
}

/**
 * @tc.name: ConvertIpv4AddressTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest002, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(ADDREDD_LEN);
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: ConvertIpv4AddressTest003
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest003, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(TEST_IP);
    ASSERT_NE(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: ConvertIpv4AddressTest004
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest004, TestSize.Level1)
{
    std::string addr;
    auto result = CommonUtils::ConvertIpv4Address(addr);
    ASSERT_EQ(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: Ipv4PrefixLenTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, Ipv4PrefixLenTest001, TestSize.Level1)
{
    std::string addr;
    auto result = CommonUtils::ConvertIpv4Address(addr);
    ASSERT_EQ(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: Ipv4PrefixLenTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, Ipv4PrefixLenTest002, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(TEST_IP);
    ASSERT_NE(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: ParseIntTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest001, TestSize.Level1)
{
    std::string testStr = "123";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_NE(value, 0);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: ParseIntTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest002, TestSize.Level1)
{
    std::string testStr = "abcdfagdshrfsth";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_EQ(value, 0);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ParseIntTest003
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest003, TestSize.Level1)
{
    std::string testStr = "44514564121561456745456891564564894";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_EQ(value, 0);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ParseIntTest004
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest004, TestSize.Level1)
{
    std::string testStr = "-156423456123512423456146";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_EQ(value, 0);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ConvertToInt64Test001
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertToInt64.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertToInt64Test001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::ConvertToInt64(testStr);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: ToAnonymousIpTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ToAnonymousIp.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ToAnonymousIpTest001, TestSize.Level1)
{
    auto result = CommonUtils::ToAnonymousIp(TEST_IPV4);
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: ToAnonymousIpTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ToAnonymousIp.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ToAnonymousIpTest002, TestSize.Level1)
{
    auto result = CommonUtils::ToAnonymousIp(TEST_IP);
    ASSERT_FALSE(result.empty());
}

/*
 * @tc.name: ToAnonymousIpTest003
 * @tc.desc: Test UtNetmanagerBaseCommon ToAnonymousIp.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ToAnonymousIpTest003, TestSize.Level1)
{
    auto result = CommonUtils::ToAnonymousIp({});
    ASSERT_TRUE(result.empty());
}

/**
 * @tc.name: StrToIntTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToIntTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToInt(testStr);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: StrToUintTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToUint.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToUintTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToUint(testStr);
    ASSERT_NE(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: StrToBoolTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToBool.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToBoolTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToBool(testStr);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: StrToLongTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToUint.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToLongTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToLong(testStr);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: IsValidDomainTest001
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest001, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN1);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidDomainTest002
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest002, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN2);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidDomainTest003
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest003, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN3);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: IsValidDomainTest004
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest004, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN4);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidDomainTest005
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest005, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN5);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidDomainTest006
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest006, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN6);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: IsValidDomainTest007
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest007, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN7);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidDomainTest008
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidDomain.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidDomainTest008, TestSize.Level1)
{
    auto result = CommonUtils::IsValidDomain(TEST_DOMAIN8);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ToLowerTest008
 * @tc.desc: Test UtNetmanagerBaseCommon ToLower.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ToLowerTest008, TestSize.Level1)
{
    std::string res = "HeLLo worLd";
    auto result = CommonUtils::ToLower(res);
    EXPECT_EQ(result, "hello world");
}

/**
 * @tc.name: GetMaskByLengthTest008
 * @tc.desc: Test UtNetmanagerBaseCommon GetMaskByLength.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, GetMaskByLengthTest008, TestSize.Level1)
{
    uint32_t length = 8;
    auto result = CommonUtils::GetMaskByLength(length);
    EXPECT_NE(result, "255.255.255.0");
}

/**
 * @tc.name: Ipv4PrefixLenTest008
 * @tc.desc: Test UtNetmanagerBaseCommon Ipv4PrefixLen.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, Ipv4PrefixLenTest008, TestSize.Level1)
{
    std::string ip = {};
    auto result = CommonUtils::Ipv4PrefixLen(ip);
    EXPECT_EQ(result, 0);
    ip = "192.168.0";
    result = CommonUtils::Ipv4PrefixLen(ip);
    EXPECT_EQ(result, 0);
    ip = "255.255.255.255";
    result = CommonUtils::Ipv4PrefixLen(ip);
    EXPECT_EQ(result, BIT_32);
    ip = "255.255.255.0";
    result = CommonUtils::Ipv4PrefixLen(ip);
    EXPECT_EQ(result, BIT_24);
    ip = "255.255.0.0";
    result = CommonUtils::Ipv4PrefixLen(ip);
    EXPECT_EQ(result, BIT_16);
    ip = "255.0.0.0";
    result = CommonUtils::Ipv4PrefixLen(ip);
    EXPECT_EQ(result, BIT_8);
    ip = "255.192.0.0";
    result = CommonUtils::Ipv4PrefixLen(ip);
    EXPECT_EQ(result, 10);
}

/**
 * @tc.name: StrToUint64Test008
 * @tc.desc: Test UtNetmanagerBaseCommon StrToUint64.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToUint64Test008, TestSize.Level1)
{
    std::string value = {};
    uint64_t defaultErr = 0;
    auto result = CommonUtils::StrToUint64(value, defaultErr);
    EXPECT_EQ(result, defaultErr);
    value = "100";
    uint64_t value2 = 100;
    result = CommonUtils::StrToUint64(value, defaultErr);
    EXPECT_EQ(result, value2);
}

HWTEST_F(UtNetmanagerBaseCommon, Trim, TestSize.Level2)
{
    std::string str = "    trim   ";
    std::string strResult = CommonUtils::Trim(str);
    EXPECT_STREQ(strResult.c_str(), "trim");
}

HWTEST_F(UtNetmanagerBaseCommon, UrlRegexParse001, TestSize.Level2)
{
    bool isMatch = CommonUtils::UrlRegexParse(TEST_DOMAIN9, TEST_DOMAIN10);
    EXPECT_EQ(isMatch, true);
}

HWTEST_F(UtNetmanagerBaseCommon, UrlRegexParse002, TestSize.Level2)
{
    bool isMatch = CommonUtils::UrlRegexParse(TEST_DOMAIN9, TEST_DOMAIN11);
    EXPECT_EQ(isMatch, false);
}

HWTEST_F(UtNetmanagerBaseCommon, UrlRegexParse003, TestSize.Level2)
{
    bool isMatch = CommonUtils::UrlRegexParse(TEST_DOMAIN9, TEST_DOMAIN12);
    EXPECT_EQ(isMatch, true);
}

HWTEST_F(UtNetmanagerBaseCommon, UrlRegexParse004, TestSize.Level2)
{
    bool isMatch = CommonUtils::UrlRegexParse(TEST_DOMAIN9, TEST_DOMAIN9);
    EXPECT_EQ(isMatch, true);
}

HWTEST_F(UtNetmanagerBaseCommon, IsUrlRegexValid001, TestSize.Level2)
{
    bool isValid = CommonUtils::IsUrlRegexValid(TEST_DOMAIN12);
    EXPECT_EQ(isValid, true);
}

HWTEST_F(UtNetmanagerBaseCommon, IsUrlRegexValid002, TestSize.Level2)
{
    bool isValid = CommonUtils::IsUrlRegexValid(TEST_DOMAIN13);
    EXPECT_EQ(isValid, false);
}

HWTEST_F(UtNetmanagerBaseCommon, GetIpv6Prefix001, TestSize.Level2)
{
    std::string ipv6Addr = TEST_IP;
    uint8_t prefixLen = MAX_IPV6_PREFIX_LENGTH;
    auto result = CommonUtils::GetIpv6Prefix(ipv6Addr, prefixLen);
    EXPECT_EQ(result, ipv6Addr);

    prefixLen = MAX_IPV6_PREFIX_LENGTH - 1;
    result = CommonUtils::GetIpv6Prefix(ipv6Addr, prefixLen);
    EXPECT_EQ(result, DEFAULT_IPV6_ANY_INIT_ADDR);
}

HWTEST_F(UtNetmanagerBaseCommon, Ipv6PrefixLen001, TestSize.Level2)
{
    std::string ip = "";
    auto result = CommonUtils::Ipv6PrefixLen(ip);
    EXPECT_EQ(result, 0);

    result = CommonUtils::Ipv6PrefixLen(TEST_IP);
    EXPECT_EQ(result, 0);
}

HWTEST_F(UtNetmanagerBaseCommon, HasInternetPermission001, TestSize.Level2)
{
    bool result = CommonUtils::HasInternetPermission();
    EXPECT_TRUE(result);
}

HWTEST_F(UtNetmanagerBaseCommon, CheckIfaceName001, TestSize.Level2)
{
    std::string name = "";
    bool result = CommonUtils::CheckIfaceName(name);
    EXPECT_FALSE(result);

    name = TEST_TEXT;
    result = CommonUtils::CheckIfaceName(name);
    EXPECT_FALSE(result);
}
} // namespace NetManagerStandard
} // namespace OHOS
