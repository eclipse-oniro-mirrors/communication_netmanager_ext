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
#include <arpa/inet.h>

#define private public
#define protected public

#include "nettrafficfilter_nfqueue_core.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr uint32_t TEST_GROUP_ID = 1001;
constexpr uint32_t TEST_PRIORITY = 100;
constexpr uint16_t TEST_QUEUE_NUM = 1001;
constexpr uint16_t DEFAULT_COPY_PACKET_LEN = 0xFFFF;
constexpr uint32_t DEFAULT_NFQUEUE_MAXLEN = 1024;
constexpr uint16_t MAX_QUEUE_NUM = 65535;
constexpr uint16_t UID = 1001;
constexpr uint16_t PID = 1001;
constexpr uint32_t COPY_MODE_META = 0;
constexpr uint32_t COPY_MODE_HEADER = 1;
constexpr uint32_t COPY_MODE_FULL = 2;
constexpr uint32_t COPY_MODE_MAXLEN = 3;

const std::string TEST_BUNDLE_NAME = "com.example.test";
const std::string TEST_BUNDLE_NAME_2 = "com.example.test2";
} // namespace

class NFQueueCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline NetTrafficFilterNFQueueCore* instance_ = nullptr;
};

void NFQueueCoreTest::SetUpTestCase() {}

void NFQueueCoreTest::TearDownTestCase() {}

void NFQueueCoreTest::SetUp()
{
    instance_ = &NetTrafficFilterNFQueueCore::GetInstance();
    instance_->isGloballyEnabled_ = true;
}

void NFQueueCoreTest::TearDown()
{
    instance_->Cleanup();
    instance_ = nullptr;
}

HWTEST_F(NFQueueCoreTest, AllocateQueueNumber, TestSize.Level1)
{
    std::string bundleName = TEST_BUNDLE_NAME;
    uint32_t groupId = TEST_GROUP_ID;
    int32_t queueNum = instance_->AllocateQueueNumber(bundleName, groupId);
    EXPECT_NE(queueNum, -1);

    QueueInfo info;
    info.bundleName = bundleName;
    info.groupId = groupId;
    instance_->queues_[queueNum] = info;
    queueNum = instance_->AllocateQueueNumber(bundleName, groupId);
    EXPECT_EQ(queueNum, -1);

    queueNum = instance_->AllocateQueueNumber(bundleName, groupId + 1);
    EXPECT_NE(queueNum, -1);

    instance_->queues_[instance_->nextQueueId_ + 1] = info;
    queueNum = instance_->AllocateQueueNumber(bundleName, groupId + 1);
    EXPECT_NE(queueNum, -1);
}

HWTEST_F(NFQueueCoreTest, GetQueueInfo, TestSize.Level1)
{
    QueueInfo info = instance_->GetQueueInfo(TEST_QUEUE_NUM);
    EXPECT_EQ(info.groupId, 0);

    info.bundleName = TEST_BUNDLE_NAME;
    info.groupId = TEST_GROUP_ID;
    instance_->queues_[TEST_QUEUE_NUM] = info;
    info = instance_->GetQueueInfo(TEST_QUEUE_NUM);
    EXPECT_EQ(info.groupId, TEST_GROUP_ID);
}

HWTEST_F(NFQueueCoreTest, GetPacketCopyLen, TestSize.Level1)
{
    OHOS::sptr<TrafficFilterConfig> config = new (std::nothrow) TrafficFilterConfig();
    config->packetCopyLen_ = DEFAULT_COPY_PACKET_LEN;
    uint32_t copyLen = instance_->GetPacketCopyLen(config, COPY_MODE_HEADER);
    EXPECT_EQ(copyLen, DEFAULT_COPY_PACKET_LEN);

    copyLen = instance_->GetPacketCopyLen(config, COPY_MODE_MAXLEN);
    EXPECT_EQ(copyLen, DEFAULT_COPY_PACKET_LEN);
}

HWTEST_F(NFQueueCoreTest, ConfigureNFQueue, TestSize.Level1)
{
    OHOS::sptr<NfqCtx> ctx = nullptr;
    OHOS::sptr<NfqQueue> qh = nullptr;
    OHOS::sptr<TrafficFilterConfig> config = new (std::nothrow) TrafficFilterConfig();
    bool ret = instance_->ConfigureNFQueue(ctx, qh, config);
    EXPECT_FALSE(ret);
}

HWTEST_F(NFQueueCoreTest, CreateQueue, TestSize.Level1)
{
    std::string bundleName = TEST_BUNDLE_NAME;
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    uint16_t queueNum = TEST_QUEUE_NUM;
    sptr<TrafficFilterConfig> config = new (std::nothrow) TrafficFilterConfig();
    config->packetCopyLen_ = DEFAULT_COPY_PACKET_LEN;
    config->nfqueueMaxlen_ = DEFAULT_NFQUEUE_MAXLEN;
    config->packetCopyMode_ = COPY_MODE_FULL;
    config->nfqueueFlags_ = 1;
    config->size_ = sizeof(TrafficFilterConfig);
    int32_t ret = instance_->CreateQueue(groupId, priority, queueNum, bundleName, config);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    instance_->DestroyQueue(queueNum);

    config->packetCopyMode_ = COPY_MODE_META;
    ret = instance_->CreateQueue(groupId, priority, queueNum, bundleName, config);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    instance_->DestroyQueue(queueNum);

    config->packetCopyMode_ = COPY_MODE_HEADER;
    ret = instance_->CreateQueue(groupId, priority, queueNum, bundleName, config);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    instance_->queues_[queueNum].qh = nullptr;
    instance_->DestroyQueue(queueNum);

    config->packetCopyMode_ = COPY_MODE_MAXLEN;
    ret = instance_->CreateQueue(groupId, priority, queueNum, bundleName, config);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    ret = instance_->CreateQueue(groupId, priority, queueNum + 1, bundleName, nullptr);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    config->nfqueueMaxlen_ = 0;
    ret = instance_->CreateQueue(groupId, priority, MAX_QUEUE_NUM, bundleName, config);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    ret = instance_->DestroyQueue(MAX_QUEUE_NUM);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    ret = instance_->DestroyByBundleName(bundleName);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NFQueueCoreTest, DestroyQueue, TestSize.Level1)
{
    uint16_t queueNum = TEST_QUEUE_NUM;
    int32_t ret = instance_->DestroyQueue(queueNum);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_NOT_FOUND);
}

HWTEST_F(NFQueueCoreTest, DestroyByBundleName, TestSize.Level1)
{
    std::string bundleName = TEST_BUNDLE_NAME;
    int32_t ret = instance_->DestroyByBundleName(bundleName);
    EXPECT_EQ(ret, TRAFFICFILTER_ERROR_NOT_FOUND);
}

HWTEST_F(NFQueueCoreTest, Observer, TestSize.Level1)
{
    std::string bundleName = "";
    uint32_t groupId = TEST_GROUP_ID;
    uint32_t priority = TEST_PRIORITY;
    uint16_t queueNum = instance_->AllocateQueueNumber(bundleName, groupId);
    instance_->HandleTrafficFilterObserverRegistration(bundleName, queueNum, UID, PID);
    EXPECT_EQ(instance_->uidToObserverMap_.size(), 0);

    bundleName = TEST_BUNDLE_NAME;
    instance_->HandleTrafficFilterObserverRegistration(bundleName, queueNum, UID, PID);
    EXPECT_EQ(instance_->uidToObserverMap_.size(), 1);

    sptr<NetTrafficFilterNFQueueCore::TrafficFilterHapObserver> observer = instance_->uidToObserverMap_[UID];
    AppExecFwk::ProcessData processData;
    processData.uid = UID + 1;
    observer->OnProcessDied(processData);
    EXPECT_EQ(instance_->uidToObserverMap_.size(), 1);

    processData.uid = UID;
    observer->OnProcessDied(processData);
    EXPECT_EQ(instance_->uidToObserverMap_.size(), 0);

    instance_->HandleTrafficFilterObserverRegistration(bundleName, queueNum, UID, PID);
    EXPECT_EQ(instance_->uidToObserverMap_.size(), 1);
    instance_->Cleanup();
    EXPECT_EQ(instance_->nextQueueId_, 0);
}

HWTEST_F(NFQueueCoreTest, PauseAllRulesEmpty, TestSize.Level1)
{
    int32_t ret = instance_->PauseAllRules();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NFQueueCoreTest, ResumeAllRulesEmpty, TestSize.Level1)
{
    int32_t ret = instance_->ResumeAllRules();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NFQueueCoreTest, PauseAllRulesWithQueue, TestSize.Level1)
{
    QueueInfo info{};
    info.groupId = TEST_GROUP_ID;
    info.priority = TEST_PRIORITY;
    info.queueNum = TEST_QUEUE_NUM;
    info.bundleName = TEST_BUNDLE_NAME;
    info.chainNameIn = "chainIn";
    info.chainNameOut = "chainOut";
    info.chainNameFwd = "chainFwd";
    instance_->queues_[TEST_QUEUE_NUM] = info;

    int32_t ret = instance_->PauseAllRules();
    EXPECT_NE(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NFQueueCoreTest, ResumeAllRulesWithQueue, TestSize.Level1)
{
    QueueInfo info{};
    info.groupId = TEST_GROUP_ID;
    info.priority = TEST_PRIORITY;
    info.queueNum = TEST_QUEUE_NUM;
    info.bundleName = TEST_BUNDLE_NAME;
    info.chainNameIn = "chainIn";
    info.chainNameOut = "chainOut";
    info.chainNameFwd = "chainFwd";
    instance_->queues_[TEST_QUEUE_NUM] = info;

    int32_t ret = instance_->ResumeAllRules();
    EXPECT_NE(ret, TRAFFICFILTER_OK);
}

HWTEST_F(NFQueueCoreTest, GlobalEnablePacketFilter001, TestSize.Level1)
{
    int32_t ret = instance_->GlobalEnablePacketFilter();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);

    instance_->isGloballyEnabled_ = false;
    ret = instance_->GlobalEnablePacketFilter();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_TRUE(instance_->isGloballyEnabled_);
}

HWTEST_F(NFQueueCoreTest, GlobalEnablePacketFilter002, TestSize.Level1)
{
    QueueInfo info{};
    info.groupId = TEST_GROUP_ID;
    info.priority = TEST_PRIORITY;
    info.queueNum = TEST_QUEUE_NUM;
    info.bundleName = TEST_BUNDLE_NAME;
    info.chainNameIn = "chainIn";
    info.chainNameOut = "chainOut";
    info.chainNameFwd = "chainFwd";
    instance_->queues_[TEST_QUEUE_NUM] = info;

    instance_->isGloballyEnabled_ = false;
    int32_t ret = instance_->GlobalEnablePacketFilter();
    EXPECT_NE(ret, TRAFFICFILTER_OK);
    EXPECT_FALSE(instance_->isGloballyEnabled_);

    instance_->queues_.clear();
    ret = instance_->GlobalEnablePacketFilter();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_TRUE(instance_->isGloballyEnabled_);
}

HWTEST_F(NFQueueCoreTest, GlobalDisablePacketFilter001, TestSize.Level1)
{
    int32_t ret = instance_->GlobalDisablePacketFilter();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_FALSE(instance_->isGloballyEnabled_);

    ret = instance_->GlobalDisablePacketFilter();
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_FALSE(instance_->isGloballyEnabled_);
}

HWTEST_F(NFQueueCoreTest, GlobalDisablePacketFilter002, TestSize.Level1)
{
    QueueInfo info{};
    info.groupId = TEST_GROUP_ID;
    info.priority = TEST_PRIORITY;
    info.queueNum = TEST_QUEUE_NUM;
    info.bundleName = TEST_BUNDLE_NAME;
    info.chainNameIn = "chainIn";
    info.chainNameOut = "chainOut";
    info.chainNameFwd = "chainFwd";
    instance_->queues_[TEST_QUEUE_NUM] = info;

    int32_t ret = instance_->GlobalDisablePacketFilter();
    EXPECT_NE(ret, TRAFFICFILTER_OK);
    EXPECT_TRUE(instance_->isGloballyEnabled_);
}

HWTEST_F(NFQueueCoreTest, GetPacketFilterGlobalStatus001, TestSize.Level1)
{
    bool isEnabled = false;
    int32_t ret = instance_->GetPacketFilterGlobalStatus(isEnabled);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_TRUE(isEnabled);
}

HWTEST_F(NFQueueCoreTest, GetPacketFilterGlobalStatus002, TestSize.Level1)
{
    instance_->GlobalDisablePacketFilter();

    bool isEnabled = true;
    int32_t ret = instance_->GetPacketFilterGlobalStatus(isEnabled);
    EXPECT_EQ(ret, TRAFFICFILTER_OK);
    EXPECT_FALSE(isEnabled);

    instance_->GlobalEnablePacketFilter();
}
} // namespace NetManagerStandard
} // namespace OHOS
