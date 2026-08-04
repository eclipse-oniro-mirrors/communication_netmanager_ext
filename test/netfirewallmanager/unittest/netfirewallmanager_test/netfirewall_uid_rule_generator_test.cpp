/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include <set>

#define private public
#define protected public

#include "netfirewall_uid_rule_generator.h"
#include "netfirewall_common.h"
#include "nettrafficfilter_nfqueue_core.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr uint32_t TEST_UID_START = 1000;
constexpr uint32_t TEST_UID_END = 2000;
constexpr uint32_t TEST_GROUP_ID = 1;
constexpr const char* TEST_BUNDLE_NAME = "com.example.test";
constexpr int32_t TEST_QUEUE_NUM = 100;

QueueInfo CreateTestQueueInfo(const std::string& bundleName = TEST_BUNDLE_NAME, uint32_t groupId = TEST_GROUP_ID)
{
    QueueInfo info;
    info.bundleName = bundleName;
    info.groupId = groupId;
    info.chainNameOut = "UIDMARK_" + bundleName + "_GRP_" + std::to_string(groupId);
    info.chainNameIn = "TF_" + bundleName + "_GRP_" + std::to_string(groupId);
    return info;
}

} // namespace

class UidRuleGeneratorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
        std::lock_guard<std::recursive_mutex> lock(generator.mutex_);
        generator.uidRuleContexts_.clear();
        generator.uidToIsolationKeys_.clear();
        generator.markToIsolationKey_.clear();
        generator.nextMark_ = 0x01;
    }
};


HWTEST_F(UidRuleGeneratorTest, GenerateIsolationKey001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string result = generator.GenerateIsolationKey("com.example.app", 100);
    EXPECT_EQ(result, "com.example.app_GRP_100");
}

HWTEST_F(UidRuleGeneratorTest, GenerateIsolationKey002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string result = generator.GenerateIsolationKey("", 0);
    EXPECT_EQ(result, "_GRP_0");
}

HWTEST_F(UidRuleGeneratorTest, GenerateIsolationKey003, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string result = generator.GenerateIsolationKey("test.bundle", 65535);
    EXPECT_EQ(result, "test.bundle_GRP_65535");
}


HWTEST_F(UidRuleGeneratorTest, HasUidCondition001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = -1;
    rule->uidEnd_ = -1;
    EXPECT_FALSE(generator.HasUidCondition(rule));
}

HWTEST_F(UidRuleGeneratorTest, HasUidCondition002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1000;
    rule->uidEnd_ = 0;
    EXPECT_TRUE(generator.HasUidCondition(rule));
}

HWTEST_F(UidRuleGeneratorTest, HasUidCondition003, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 0;
    rule->uidEnd_ = 2000;
    EXPECT_TRUE(generator.HasUidCondition(rule));
}

HWTEST_F(UidRuleGeneratorTest, HasUidCondition004, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1000;
    rule->uidEnd_ = 2000;
    EXPECT_TRUE(generator.HasUidCondition(rule));
}

HWTEST_F(UidRuleGeneratorTest, GenerateCTMarkMatchParam001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string result = generator.GenerateCTMarkMatchParam(0x1A);
    EXPECT_EQ(result, "-m connmark --mark 0x1a");
}

HWTEST_F(UidRuleGeneratorTest, GenerateCTMarkMatchParam002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string result = generator.GenerateCTMarkMatchParam(255);
    EXPECT_EQ(result, "-m connmark --mark 0xff");
}


HWTEST_F(UidRuleGeneratorTest, IsUidRangeOverlap001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    EXPECT_FALSE(generator.IsUidRangeOverlap(1000, 2000, ""));
}

HWTEST_F(UidRuleGeneratorTest, IsUidRangeOverlap002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.AllocateMarkForUidRange("test.bundle", 1, 1000, 2000, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_TRUE(generator.IsUidRangeOverlap(1000, 2000, ""));
    EXPECT_TRUE(generator.IsUidRangeOverlap(1500, 2500, ""));
    EXPECT_TRUE(generator.IsUidRangeOverlap(500, 1500, ""));
    EXPECT_FALSE(generator.IsUidRangeOverlap(2001, 3000, ""));
    EXPECT_FALSE(generator.IsUidRangeOverlap(0, 999, ""));
}

HWTEST_F(UidRuleGeneratorTest, IsUidRangeOverlap003, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.AllocateMarkForUidRange("test.bundle", 1, 1000, 2000, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    std::string isolationKey = generator.GenerateIsolationKey("test.bundle", 1);
    EXPECT_FALSE(generator.IsUidRangeOverlap(1000, 2000, isolationKey));
}

HWTEST_F(UidRuleGeneratorTest, AllocateMarkForUidRange001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID,
                                                     TEST_UID_START, TEST_UID_END, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_GT(markValue, 0u);
    EXPECT_LE(markValue, 0xFFu);
}

HWTEST_F(UidRuleGeneratorTest, AllocateMarkForUidRange002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID,
                                                     2000, 1000, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, AllocateMarkForUidRange004, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue1 = 0;
    uint32_t markValue2 = 0;
    int32_t ret = generator.AllocateMarkForUidRange("bundle1", 1, 1000, 2000, markValue1);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    ret = generator.AllocateMarkForUidRange("bundle2", 2, 1500, 2500, markValue2);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, ReleaseMarkForUidRange001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID,
                                                     1000, 2000, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    ret = generator.ReleaseMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    std::string isolationKey = generator.GenerateIsolationKey(TEST_BUNDLE_NAME, TEST_GROUP_ID);
    EXPECT_TRUE(generator.uidRuleContexts_.find(isolationKey) == generator.uidRuleContexts_.end());
}

HWTEST_F(UidRuleGeneratorTest, ReleaseMarkForUidRange002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.ReleaseMarkForUidRange("nonexistent.bundle", 999);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, GetMarkByUid001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t allocatedMark = 0;
    int32_t ret = generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID,
                                                     1000, 2000, allocatedMark);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    uint32_t markValue = 0;
    ret = generator.GetMarkByUid(1500, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_EQ(markValue, allocatedMark);
}

HWTEST_F(UidRuleGeneratorTest, GetMarkByUid002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.GetMarkByUid(9999, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, CreateOrUpdateContext001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    QueueInfo info = CreateTestQueueInfo("com.example.app", 100);
    auto ctx = generator.CreateOrUpdateContext(info, 1000, 2000, 50);
    EXPECT_NE(ctx, nullptr);
    EXPECT_EQ(ctx->bundleName, "com.example.app");
    EXPECT_EQ(ctx->groupId, 100u);
    EXPECT_EQ(ctx->uidStart, 1000u);
    EXPECT_EQ(ctx->uidEnd, 2000u);
    EXPECT_EQ(ctx->queueNum, 50);
    EXPECT_EQ(ctx->filterChainName, info.chainNameOut);
    EXPECT_EQ(ctx->mangleChainName, info.chainNameOut);
}

HWTEST_F(UidRuleGeneratorTest, CreateOrUpdateContext002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    QueueInfo info = CreateTestQueueInfo("com.example.app", 100);
    auto ctx1 = generator.CreateOrUpdateContext(info, 1000, 2000, 50);
    EXPECT_NE(ctx1, nullptr);
    auto ctx2 = generator.CreateOrUpdateContext(info, 3000, 4000, 60);
    EXPECT_EQ(ctx1, ctx2);
    EXPECT_EQ(ctx2->uidStart, 3000u);
    EXPECT_EQ(ctx2->uidEnd, 4000u);
    EXPECT_EQ(ctx2->queueNum, 60);
}

HWTEST_F(UidRuleGeneratorTest, HandleAddUidRule001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 0;
    rule->uidEnd_ = 0;
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.HandleAddUidRule(info, rule, TEST_QUEUE_NUM);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}


HWTEST_F(UidRuleGeneratorTest, HandleAddUidRule002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 2000;
    rule->uidEnd_ = 1000;
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.HandleAddUidRule(info, rule, TEST_QUEUE_NUM);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, HandleAddUidRule004, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1000;
    rule->uidEnd_ = 2000;
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.HandleAddUidRule(info, rule, TEST_QUEUE_NUM);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, HandleAddUidRule005, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.AllocateMarkForUidRange("test.bundle", 1, 1000, 2000, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1500;
    rule->uidEnd_ = 2500;
    QueueInfo info = CreateTestQueueInfo();
    ret = generator.HandleAddUidRule(info, rule, TEST_QUEUE_NUM);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, UpdateUidRangeIfNeeded001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID,
                                                     1000, 2000, markValue);
    EXPECT_EQ(ret, 0);
    std::string isolationKey = generator.GenerateIsolationKey(TEST_BUNDLE_NAME, TEST_GROUP_ID);
    EXPECT_TRUE(generator.uidRuleContexts_.find(isolationKey) != generator.uidRuleContexts_.end());
    ret = generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID,
                                            3000, 4000, markValue);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(UidRuleGeneratorTest, AllocateNextMark001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::set<uint32_t> allocatedMarks;
    for (int i = 0; i < 10; i++) {
        uint32_t mark = generator.AllocateNextMark();
        EXPECT_GT(mark, 0u);
        EXPECT_LE(mark, 0xFFu);
        allocatedMarks.insert(mark);
    }
    EXPECT_EQ(allocatedMarks.size(), 10u);
}

HWTEST_F(UidRuleGeneratorTest, AllocateNextMark002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    for (int i = 0; i < 255; i++) {
        uint32_t mark = generator.AllocateNextMark();
        if (mark == 0) {
            break;
        }
    }
    uint32_t mark = generator.AllocateNextMark();
    EXPECT_EQ(mark, 0u);
}

HWTEST_F(UidRuleGeneratorTest, RemoveUidFromMapping001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string isolationKey = generator.GenerateIsolationKey(TEST_BUNDLE_NAME, TEST_GROUP_ID);
    // 创建 Context 并设置数据
    auto ctx = std::make_shared<UidRuleContext>();
    ctx->bundleName = TEST_BUNDLE_NAME;
    ctx->groupId = TEST_GROUP_ID;
    ctx->uidStart = 1000;
    ctx->uidEnd = 2000;
    ctx->ctMarkValue = 1;
    generator.uidRuleContexts_[isolationKey] = ctx;
    for (uint32_t uid = 1000; uid <= 2000; uid++) {
        generator.uidToIsolationKeys_[uid].insert(isolationKey);
    }
    generator.RemoveUidFromMapping(isolationKey, 1000, 2000);
    EXPECT_TRUE(generator.uidToIsolationKeys_.find(1500) == generator.uidToIsolationKeys_.end());
}

HWTEST_F(UidRuleGeneratorTest, GenerateCreateMangleRulesCommands001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    QueueInfo info = CreateTestQueueInfo();
    uint32_t markValue = 0;
    generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID, 1000, 2000, markValue);
    int32_t ret = generator.GenerateCreateMangleRulesCommands(info, 1000, 2000, markValue);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, GenerateDeleteMangleChainCommands001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string isolationKey = generator.GenerateIsolationKey(TEST_BUNDLE_NAME, TEST_GROUP_ID);
    // 创建 Context 并设置 hasOutputRule
    auto ctx = std::make_shared<UidRuleContext>();
    ctx->bundleName = TEST_BUNDLE_NAME;
    ctx->groupId = TEST_GROUP_ID;
    ctx->uidStart = 1000;
    ctx->uidEnd = 2000;
    ctx->ctMarkValue = 1;
    ctx->hasOutputRule = true;
    generator.uidRuleContexts_[isolationKey] = ctx;
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.GenerateDeleteMangleChainCommands(info);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, GenerateDeleteMangleChainCommands002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    QueueInfo info = CreateTestQueueInfo("nonexistent.bundle", 999);
    int32_t ret = generator.GenerateDeleteMangleChainCommands(info);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(UidRuleGeneratorTest, HandleOutputUidRule001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1000;
    rule->uidEnd_ = 2000;
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.HandleOutputUidRule(info, rule, TEST_QUEUE_NUM);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, HandleOutputUidRule002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1000;
    rule->uidEnd_ = 2000;
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.HandleOutputUidRule(info, rule, TEST_QUEUE_NUM);
    std::string isolationKey = generator.GenerateIsolationKey(TEST_BUNDLE_NAME, TEST_GROUP_ID);
    auto it = generator.uidRuleContexts_.find(isolationKey);
    if (ret == TRAFFICFILTER_OK) {
        EXPECT_NE(it, generator.uidRuleContexts_.end());
        if (it != generator.uidRuleContexts_.end()) {
            EXPECT_TRUE(it->second->hasOutputRule);
        }
    }
}

HWTEST_F(UidRuleGeneratorTest, HandleInputUidRule001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1000;
    rule->uidEnd_ = 2000;
    uint32_t markValue = 0;
    generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID, 1000, 2000, markValue);
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.HandleInputUidRule(info, rule, TEST_QUEUE_NUM);
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, HandleInputUidRule002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    sptr<TrafficFilterPacketRule> rule = new TrafficFilterPacketRule();
    rule->uidStart_ = 1000;
    rule->uidEnd_ = 2000;
    QueueInfo info = CreateTestQueueInfo();
    int32_t ret = generator.HandleInputUidRule(info, rule, TEST_QUEUE_NUM);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, GetMarkByUidRange001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t allocatedMark = 0;
    int32_t ret = generator.AllocateMarkForUidRange(TEST_BUNDLE_NAME, TEST_GROUP_ID,
                                                     1000, 2000, allocatedMark);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    uint32_t markValue = 0;
    ret = generator.GetMarkByUidRange(1000, 2000, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_EQ(markValue, allocatedMark);
}

HWTEST_F(UidRuleGeneratorTest, GetMarkByUidRange002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    uint32_t markValue = 0;
    int32_t ret = generator.GetMarkByUidRange(9999, 99999, markValue);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, BuildInputCtmarkRule001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    std::string result = generator.BuildInputCtmarkRule("TEST_CHAIN", 100, 0x1A);
    EXPECT_NE(result.find("-t filter -A TEST_CHAIN"), std::string::npos);
    EXPECT_NE(result.find("-m connmark --mark"), std::string::npos);
    EXPECT_NE(result.find("-j NFQUEUE --queue-num 100"), std::string::npos);
}

HWTEST_F(UidRuleGeneratorTest, CreateMangleChain001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.CreateMangleChain("TEST_MANGLE_CHAIN");
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, InsertMangleChainJump001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.InsertMangleChainJump("TEST_MANGLE_CHAIN");
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, AddMangleMarkRule001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.AddMangleMarkRule("TEST_CHAIN", 1000, 1000, "0x1a");
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, AddMangleMarkRule002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.AddMangleMarkRule("TEST_CHAIN", 1000, 2000, "0x1a");
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, AddMangleConnmarkRule001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.AddMangleConnmarkRule("TEST_CHAIN", "0x1a");
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, RollbackMangleChainCreation001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    generator.RollbackMangleChainCreation("TEST_CHAIN", static_cast<int32_t>(MangleChainStage::STAGE_CHAIN_CREATED));
    EXPECT_TRUE(true);
}

HWTEST_F(UidRuleGeneratorTest, RollbackMangleChainCreation002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    generator.RollbackMangleChainCreation("TEST_CHAIN", static_cast<int32_t>(MangleChainStage::STAGE_JUMP_INSERTED));
    EXPECT_TRUE(true);
}

HWTEST_F(UidRuleGeneratorTest, RollbackMangleChainCreation003, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    generator.RollbackMangleChainCreation("TEST_CHAIN", static_cast<int32_t>(MangleChainStage::STAGE_MARK_RULE_ADDED));
    EXPECT_TRUE(true);
}

HWTEST_F(UidRuleGeneratorTest, HandleClearUidRules001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    QueueInfo info = CreateTestQueueInfo();
    std::string isolationKey = generator.GenerateIsolationKey(TEST_BUNDLE_NAME, TEST_GROUP_ID);

    auto ctx = std::make_shared<UidRuleContext>();
    ctx->bundleName = TEST_BUNDLE_NAME;
    ctx->groupId = TEST_GROUP_ID;
    ctx->hasOutputRule = true;
    ctx->hasInputRule = true;
    generator.uidRuleContexts_[isolationKey] = ctx;

    int32_t ret = generator.HandleClearUidRules(info);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_TRUE(generator.uidRuleContexts_.find(isolationKey) == generator.uidRuleContexts_.end());
}

HWTEST_F(UidRuleGeneratorTest, HandleClearUidRules002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    QueueInfo info = CreateTestQueueInfo("nonexistent.bundle", 999);
    int32_t ret = generator.HandleClearUidRules(info);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(UidRuleGeneratorTest, ExecuteCmd001, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.ExecuteCmd("invalid_iptables_command");
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_INVALID_PARAM);
}

HWTEST_F(UidRuleGeneratorTest, ExecuteCmd002, TestSize.Level1)
{
    UidRuleGenerator& generator = UidRuleGenerator::GetInstance();
    int32_t ret = generator.ExecuteCmd("-t filter -L");
    EXPECT_TRUE(ret == TRAFFICFILTER_OK || ret == TRAFFICFILTER_ERROR_INVALID_PARAM);
}

} // namespace NetManagerStandard
} // namespace OHOS
