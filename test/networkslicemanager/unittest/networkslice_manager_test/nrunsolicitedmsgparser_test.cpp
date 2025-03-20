/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <gtest/gtest.h>
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "nrunsolicitedmsgparser.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_client.h"
 
namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
 
class NrunsolicitedmsgparserTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
 
void NrunsolicitedmsgparserTest::SetUpTestCase() {}
 
void NrunsolicitedmsgparserTest::TearDownTestCase() {}
 
void NrunsolicitedmsgparserTest::SetUp() {}
 
void NrunsolicitedmsgparserTest::TearDown() {}
 
constexpr int DECODE_FAIL_UNKNOWN_IDENTIFIER = 1;
constexpr int DECODE_SUCCESS = 0;
constexpr int DECODE_FAIL_OTHER = -1;
 
HWTEST_F(NrunsolicitedmsgparserTest, GetInstance001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetInstance001");
    NrUnsolicitedMsgParser::GetInstance();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetSubscriberIdAndUrspFromFile002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetSubscriberIdAndUrspFromFile002");
    NrUnsolicitedMsgParser::GetInstance().GetSubscriberIdAndUrspFromFile();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, HandleSimStateChanged002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleSimStateChanged002");
    NrUnsolicitedMsgParser::GetInstance().HandleSimStateChanged();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SyncSubscriberId001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SyncSubscriberId001");
    NrUnsolicitedMsgParser::GetInstance().SyncSubscriberId();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetNetworkSliceAllowedNssai001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetNetworkSliceAllowedNssai001");
    NrUnsolicitedMsgParser::GetInstance().GetNetworkSliceAllowedNssai();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetNetworkSliceEhplmn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetNetworkSliceEhplmn001");
    NrUnsolicitedMsgParser::GetInstance().GetNetworkSliceEhplmn();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, WriteObjectToJsonFile001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("WriteObjectToJsonFile001");
    const std::string fileName;
    std::unordered_map<std::string, UePolicy> obj;
    NrUnsolicitedMsgParser::GetInstance().WriteObjectToJsonFile(fileName, obj);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, ReadObjectFromJsonFile002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("ReadObjectFromJsonFile002");
    std::string fileName = "1";
    std::ifstream file(fileName);
    NrUnsolicitedMsgParser::GetInstance().ReadObjectFromJsonFile(fileName);
    EXPECT_TRUE(!file.is_open());
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetHplmn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetHplmn001");
    NrUnsolicitedMsgParser::GetInstance().GetHplmn();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetAllowedNssaiFromUnsolData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetAllowedNssaiFromUnsolData001");
    std::vector<uint8_t> buffer = {};
    NrUnsolicitedMsgParser::GetInstance().GetAllowedNssaiFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetEhplmnFromUnsolData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetEhplmnFromUnsolData001");
    std::vector<uint8_t> buffer = {};
    NrUnsolicitedMsgParser::GetInstance().GetEhplmnFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetUrspFromUnsolData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUrspFromUnsolData001");
    std::vector<uint8_t> buffer = {};
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetUrspFromUnsolData002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUrspFromUnsolData002");
    std::vector<uint8_t> buffer = {0x01};
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetUrspFromUnsolData003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUrspFromUnsolData003");
    std::vector<uint8_t> buffer = {0x01, 0x00, 0x01};
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetUrspFromUnsolData004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUrspFromUnsolData004");
    std::vector<uint8_t> buffer = {0x01, 0x01, 0x01};
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetUrspFromUnsolData005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUrspFromUnsolData005");
    std::vector<uint8_t> buffer = {0x01, 0x02, 0x03, 0x05, 0xE6, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetUrspFromUnsolData006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUrspFromUnsolData006");
    std::vector<uint8_t> buffer = {0x02, 0x03, 0x02, 0x05, 0xE6, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, GetUrspFromUnsolData007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetUrspFromUnsolData007");
    std::vector<uint8_t> buffer = {0x02, 0x04, 0x02, 0x05, 0xE6, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    NrUnsolicitedMsgParser::GetInstance().GetUrspFromUnsolData(buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData000, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData000");
    std::vector<uint8_t> buffer = {0x05, 0xE6, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int startIndex = 0;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData002");
    std::vector<uint8_t> buffer = {};
    int startIndex = 0;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData003");
    std::vector<uint8_t> buffer = {0x05, 0xE6, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int startIndex = 32;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData004");
    std::vector<uint8_t> buffer = {0x05, 0xE7, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int startIndex = 0;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData005");
    std::vector<uint8_t> buffer = {0x05, 0xE6, 0x01, 0x01, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x05, 0xE6};
    int startIndex = 30;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData006");
    std::vector<uint8_t> buffer = {0x05, 0xE6, 0x01, 0x02, 0x00, 0x1A,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int startIndex = 0;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData007, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData007");
    std::vector<uint8_t> buffer = {0x05, 0xE6, 0x01, 0x01, 0x00, 0x19,
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int startIndex = 0;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, HandleDecodeResult002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleDecodeResult002");
    uint8_t pti = 0;
    std::string plmn = {};
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    NrUnsolicitedMsgParser::GetInstance().HandleDecodeResult(pti, decodeUePolicyMap);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SndManageUePolicyComplete001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SndManageUePolicyComplete001");
    uint8_t pti = 0;
    NrUnsolicitedMsgParser::GetInstance().SndManageUePolicyComplete(pti);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, AddNewUePolicy001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("AddNewUePolicy001");
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    NrUnsolicitedMsgParser::GetInstance().AddNewUePolicy(decodeUePolicyMap);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, isUePolicyLegal001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isUePolicyLegal001");
    std::string plmn = {};
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    UePolicyRejectMsg uePolicyRejectMsg;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().isUePolicyLegal(decodeUePolicyMap, uePolicyRejectMsg), true);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, isPlmnInHplmn002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isPlmnInHplmn002");
    std::string plmn = "";
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().isPlmnInHplmn(plmn), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, isPlmnInHplmn001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isPlmnInHplmn001");
    std::string plmn = "A";
    std::vector<uint8_t> buffer = {0x41, 0x2C, 0x42};
    NrUnsolicitedMsgParser::GetInstance().GetEhplmnFromUnsolData(buffer);
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().isPlmnInHplmn(plmn), true);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, isPlmnInHplmn003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("isPlmnInHplmn003");
    std::string plmn = "";
    std::vector<uint8_t> buffer = {0x41, 0x2C, 0x42};
    NrUnsolicitedMsgParser::GetInstance().GetEhplmnFromUnsolData(buffer);
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().isPlmnInHplmn(plmn), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySectionList001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySectionList001");
    std::vector<uint8_t> buffer = {0x00, 0x1A, 0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01,
    0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySectionList(
        inputLen, startIndex, buffer, decodeUePolicyMap), true);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySectionList002, testing::ext::TestSize.Level1) // false
{
    NETMGR_EXT_LOG_I("DecodePolicySectionList002");
    std::vector<uint8_t> buffer = {0x00, 0x1A, 0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01,
    0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size() - 1;
    int startIndex = 0;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySectionList(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySectionList003, testing::ext::TestSize.Level1) // false
{
    NETMGR_EXT_LOG_I("DecodePolicySectionList003");
    std::vector<uint8_t> buffer = {0x00, 0x1A, 0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01,
    0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x00, 0x1A};
    int inputLen = buffer.size();
    int startIndex = 26;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySectionList(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySectionList004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySectionList004");
    std::vector<uint8_t> buffer = {0x00, 0x1A, 0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x1A, 0x01,
    0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 11;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySectionList(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySectionList005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySectionList005");
    std::vector<uint8_t> buffer = {0x1A, 0x00, // len
        0x00, 0x18, 0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x00, 0x00};
    int inputLen = buffer.size() - 24;
    int startIndex = 24;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySectionList(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySection001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySection001");
    std::vector<uint8_t> buffer = {0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00,
    0X02, 0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySection(
        inputLen, startIndex, buffer, decodeUePolicyMap), true);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySection002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySection002");
    std::vector<uint8_t> buffer = {0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02,
    0x00, 0X02, 0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size() - 24;
    int startIndex = 0;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySection(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySection004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySection004");
    std::vector<uint8_t> buffer = {0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02,
    0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 24;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySection(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}

HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySection005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySection005");
    std::vector<uint8_t> buffer = {0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02,
    0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 10;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySection(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodePolicySection006, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodePolicySection006");
    std::vector<uint8_t> buffer = {0x00, 0x01, 0x01, 0x00, 0x13, 0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02,
    0x30, 0x16, 0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size() + 21;
    int startIndex = 0;
    std::unordered_map<std::string, UePolicy> decodeUePolicyMap;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodePolicySection(
        inputLen, startIndex, buffer, decodeUePolicyMap), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, PlmnToString001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("PlmnToString001");
    std::vector<uint8_t> plmns = {0x00, 0x01, 0x01};
    string plmn = NrUnsolicitedMsgParser::GetInstance().PlmnToString(plmns);
    EXPECT_EQ(plmn, "001100");
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeInstruction001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeInstruction001");
    std::vector<uint8_t> buffer = {0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    UePolicy uePolicy;
    short instructionOrder = 0;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeInstruction(
        inputLen, startIndex, buffer, uePolicy, instructionOrder), true);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeInstruction002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeInstruction002");
    std::vector<uint8_t> buffer = {0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size() - 19;
    int startIndex = 0;
    UePolicy uePolicy;
    short instructionOrder = 0;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeInstruction(
        inputLen, startIndex, buffer, uePolicy, instructionOrder), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeInstruction003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeInstruction003");
    std::vector<uint8_t> buffer = {0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 19;
    UePolicy uePolicy;
    short instructionOrder = 0;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeInstruction(
        inputLen, startIndex, buffer, uePolicy, instructionOrder), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeInstruction004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeInstruction004");
    std::vector<uint8_t> buffer = {0x03, 0x09, 0x00, 0x0F, 0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 10;
    UePolicy uePolicy;
    short instructionOrder = 0;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeInstruction(
        inputLen, startIndex, buffer, uePolicy, instructionOrder), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeInstruction005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeInstruction005");
    std::vector<uint8_t> buffer = {0x03, 0x09, 0x00, 0x0F, 0x01, 0x00, 0x0E, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x07, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    UePolicy uePolicy;
    short instructionOrder = 0;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeInstruction(
        inputLen, startIndex, buffer, uePolicy, instructionOrder), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUePolicyPart001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUePolicyPart001");
    std::vector<uint8_t> buffer = {0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16, // ProtocolId
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03}; // SscMode
    int inputLen = buffer.size();
    int startIndex = 0;
    PolicyInstruction policyInstruction;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeUePolicyPart(
        inputLen, startIndex, buffer, policyInstruction), true);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUePolicyPart002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUePolicyPart002");
    std::vector<uint8_t> buffer = {0x01, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size() - 15;
    int startIndex = 0;
    PolicyInstruction policyInstruction;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeUePolicyPart(
        inputLen, startIndex, buffer, policyInstruction), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUePolicyPart003, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUePolicyPart003");
    std::vector<uint8_t> buffer = {0x00, 0x0D, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x06, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    PolicyInstruction policyInstruction;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeUePolicyPart(
        inputLen, startIndex, buffer, policyInstruction), true);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUePolicyPart005, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUePolicyPart005");
    std::vector<uint8_t> buffer = {0x01, 0x0E, 0x00, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x07, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    PolicyInstruction policyInstruction;
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeUePolicyPart(
        inputLen, startIndex, buffer, policyInstruction), false);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SendUePolicySectionIdentifier002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SendUePolicySectionIdentifier002");
    NrUnsolicitedMsgParser::GetInstance().SendUePolicySectionIdentifier();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SendImsRsdList002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SndImsRsdList002");
    NrUnsolicitedMsgParser::GetInstance().SendImsRsdList();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SendUrspUpdate002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SendUrspUpdate002");
    NrUnsolicitedMsgParser::GetInstance().SendUrspUpdate();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, UpdateUrspRules002, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UpdateUrspRules002");
    NrUnsolicitedMsgParser::GetInstance().UpdateUrspRules();
}
 
// extern std::shared_ptr<UrspConfig> sUrspConfig_;
HWTEST_F(NrunsolicitedmsgparserTest, UpdateUrspRules001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("UpdateUrspRules001");
    sUrspConfig_.reset();
    NrUnsolicitedMsgParser::GetInstance().UpdateUrspRules();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SendUrspUpdate001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SendUrspUpdate001");
    NrUnsolicitedMsgParser::GetInstance().SendUrspUpdate();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUrspFromUnsolData001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUrspFromUnsolData001");
    std::vector<uint8_t> buffer = {};
    int startIndex = 0;
    NrUnsolicitedMsgParser::GetInstance().DecodeUrspFromUnsolData(startIndex, buffer);
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SendUePolicySectionIdentifier001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SendUePolicySectionIdentifier001");
    NrUnsolicitedMsgParser::GetInstance().SendUePolicySectionIdentifier();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, SendImsRsdList001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("SendImsRsdList001");
    NrUnsolicitedMsgParser::GetInstance().SendImsRsdList();
}
 
HWTEST_F(NrunsolicitedmsgparserTest, DecodeUePolicyPart004, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("DecodeUePolicyPart004");
    std::vector<uint8_t> buffer = {0x01, 0x0E, 0x00, 0x02, 0x00, 0X02, 0x30, 0x16,
        0x00, 0x07, 0x05, 0x03, 0x00, 0x02, 0x01, 0x03};
    int inputLen = buffer.size();
    int startIndex = 0;
    PolicyInstruction policyInstruction;
    sUrspConfig_.reset();
    EXPECT_EQ(NrUnsolicitedMsgParser::GetInstance().DecodeUePolicyPart(
        inputLen, startIndex, buffer, policyInstruction), false);
}
 
}
}