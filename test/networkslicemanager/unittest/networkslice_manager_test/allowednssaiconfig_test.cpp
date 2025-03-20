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
#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <gtest/gtest.h>
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_client.h"
#include "state_utils.h"
 
namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
 
class AllowedNssaiConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
 
void AllowedNssaiConfigTest::SetUpTestCase() {}
 
void AllowedNssaiConfigTest::TearDownTestCase() {}
 
void AllowedNssaiConfigTest::SetUp() {}
 
void AllowedNssaiConfigTest::TearDown() {}
 
constexpr int DECODE_FAIL_UNKNOWN_IDENTIFIER = 1;
constexpr int DECODE_SUCCESS = 0;
constexpr int DECODE_FAIL_OTHER = -1;

HWTEST_F(AllowedNssaiConfigTest, DecodeAllowedNssai001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeAllowedNssai001");
    std::vector<uint8_t> buffer = {0x41, 0x2E, 0x42, 0x3A, 0x43, 0x2E, 0x44}; // A.B:C.D
    AllowedNssaiConfig::GetInstance().DecodeAllowedNssai(buffer);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeAllowedNssai002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeAllowedNssai002");
    std::vector<uint8_t> buffer = {};
    AllowedNssaiConfig::GetInstance().DecodeAllowedNssai(buffer);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeAllowedNssai003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeAllowedNssai003");
    std::vector<uint8_t> buffer = {0x41, 0x2E, 0x42, 0x2E, 0x43, 0x2E, 0x44}; // A.B.C.D
    AllowedNssaiConfig::GetInstance().DecodeAllowedNssai(buffer);
}
 
HWTEST_F(AllowedNssaiConfigTest, ParseSnssai002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ParseSnssai002");
    Snssai snssai;
    snssai.setSnssai("");
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().ParseSnssai(snssai), false);
}
 
HWTEST_F(AllowedNssaiConfigTest, ParseSnssai003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ParseSnssai003");
    Snssai snssai;
    snssai.setSnssai("A;B;C");
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().ParseSnssai(snssai), false);
}
 
HWTEST_F(AllowedNssaiConfigTest, ParseSnssai004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ParseSnssai004");
    Snssai snssai;
    snssai.setSnssai("A.B.C.D");
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().ParseSnssai(snssai), false);
}
 
HWTEST_F(AllowedNssaiConfigTest, ParseSnssai005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ParseSnssai005");
    Snssai snssai;
    snssai.setSnssai("A;B.C.D");
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().ParseSnssai(snssai), false);
}
 
HWTEST_F(AllowedNssaiConfigTest, ParseSnssai006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ParseSnssai006");
    Snssai snssai;
    snssai.setSnssai("A;B");
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().ParseSnssai(snssai), true);
}
 
HWTEST_F(AllowedNssaiConfigTest, ParseSnssai007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ParseSnssai007");
    Snssai snssai;
    snssai.setSnssai("A;B.C");
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().ParseSnssai(snssai), true);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeSnssai001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai001");
    std::vector<uint8_t> buffer = {0x41};
    int startIndex = 0;
    uint8_t len = buffer.size();
    Snssai snssai;
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().DecodeSnssai(startIndex, len, buffer, snssai), true);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeSnssai002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai002");
    std::vector<uint8_t> buffer = {0x00, 0x00};
    int startIndex = 0;
    uint8_t len = buffer.size();
    Snssai snssai;
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().DecodeSnssai(startIndex, len, buffer, snssai), true);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeSnssai003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai003");
    std::vector<uint8_t> buffer = {0x00, 0x00, 0x00};
    int startIndex = 0;
    uint8_t len = buffer.size();
    Snssai snssai;
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().DecodeSnssai(startIndex, len, buffer, snssai), false);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeSnssai004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai003");
    std::vector<uint8_t> buffer = {0x00, 0x00, 0x00, 0x00};
    int startIndex = 0;
    uint8_t len = buffer.size();
    Snssai snssai;
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().DecodeSnssai(startIndex, len, buffer, snssai), true);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeSnssai005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai003");
    std::vector<uint8_t> buffer = {0x00, 0x00, 0x00, 0x00, 0x00};
    int startIndex = 0;
    uint8_t len = buffer.size();
    Snssai snssai;
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().DecodeSnssai(startIndex, len, buffer, snssai), true);
}
 
HWTEST_F(AllowedNssaiConfigTest, DecodeSnssai006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeSnssai003");
    std::vector<uint8_t> buffer = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int startIndex = 0;
    uint8_t len = buffer.size();
    Snssai snssai;
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().DecodeSnssai(startIndex, len, buffer, snssai), true);
}
 
HWTEST_F(AllowedNssaiConfigTest, FindSnssaiInAllowedNssai002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindSnssaiInAllowedNssai002");
    std::vector<uint8_t> buffer = {0x41, 0x2E, 0x42, 0x3A, 0x43, 0x2E, 0x44}; // A.B:C.D
    AllowedNssaiConfig::GetInstance().DecodeAllowedNssai(buffer);
    std::vector<Snssai> snssais;
    Snssai snssai;
    snssai.setSnssaiLen(4);
    snssai.setSst(0x0A);
    snssai.setSd(0x0B);
    snssais.push_back(snssai);
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().FindSnssaiInAllowedNssai(snssais), "A.B");
}
 
HWTEST_F(AllowedNssaiConfigTest, FindSnssaiInAllowedNssai003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("FindSnssaiInAllowedNssai003");
    std::vector<Snssai> snssais = {};
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().FindSnssaiInAllowedNssai(snssais), "");
}

HWTEST_F(AllowedNssaiConfigTest, isSnssaiInAllowedNssai002, testing::ext::TestSize.Level1) // "A.B"
{
    NETMGR_EXT_LOG_I("isSnssaiInAllowedNssai002");
    std::vector<uint8_t> buffer = {0x41, 0x2E, 0x42, 0x3A, 0x43, 0x2E, 0x44}; // A.B:C.D
    AllowedNssaiConfig::GetInstance().DecodeAllowedNssai(buffer);
    std::vector<Snssai> snssais;
    Snssai snssai;
    snssai.setSnssaiLen(4);
    snssai.setSst(0x0A);
    snssai.setSd(0x0B);
    snssais.push_back(snssai);
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().isSnssaiInAllowedNssai(snssai), "A.B");
}
 
HWTEST_F(AllowedNssaiConfigTest, isSnssaiInAllowedNssai003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isSnssaiInAllowedNssai003");
    Snssai snssai = {0x01, 0x03};
    EXPECT_EQ(AllowedNssaiConfig::GetInstance().isSnssaiInAllowedNssai(snssai), "");
}
 
}
}