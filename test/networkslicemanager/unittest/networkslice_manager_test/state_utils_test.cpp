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
#include <gtest/gtest.h>
#include "state_utils.h"
#include "networksliceutil.h"

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;
constexpr int32_t MODEM_ID_0 = 0x00;
constexpr int32_t MODEM_ID_1 = 0x01;
class StateUtilsTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(StateUtilsTest, GetNowMilliSeconds001, testing::ext::TestSize.Level1)
{
    int64_t time = StateUtils::GetNowMilliSeconds();
    EXPECT_GE(time, 0LL);
}

HWTEST_F(StateUtilsTest, GetCurrentSysTimeMs001, testing::ext::TestSize.Level1)
{
    int64_t time = StateUtils::GetCurrentSysTimeMs();
    EXPECT_GE(time, 0LL);
}

HWTEST_F(StateUtilsTest, GetDefaultSlotId001, testing::ext::TestSize.Level1)
{
    int32_t defaultSlot = StateUtils::GetDefaultSlotId();
    EXPECT_EQ(defaultSlot, DEFAULT_SLOT);
}

HWTEST_F(StateUtilsTest, GetSlaveCardSlotId001, testing::ext::TestSize.Level1)
{
    int32_t SlaveSlot = StateUtils::GetSlaveCardSlotId();
    EXPECT_NE(SlaveSlot, DEFAULT_SLOT);
}

HWTEST_F(StateUtilsTest, GetModemIdBySlotId001, testing::ext::TestSize.Level1)
{
    int32_t modemId = StateUtils::GetModemIdBySlotId(DEFAULT_SLOT);
    EXPECT_EQ(modemId, MODEM_ID_0);
}

HWTEST_F(StateUtilsTest, GetModemIdBySlotId002, testing::ext::TestSize.Level1)
{
    int32_t modemId = StateUtils::GetModemIdBySlotId(1);
    EXPECT_EQ(modemId, MODEM_ID_1);
}

HWTEST_F(StateUtilsTest, GetModemIdBySlotId003, testing::ext::TestSize.Level1)
{
    int32_t modemId = StateUtils::GetModemIdBySlotId(2);
    EXPECT_EQ(modemId, MODEM_ID_0);
}

HWTEST_F(StateUtilsTest, GetSlotIdByModemId001, testing::ext::TestSize.Level1)
{
    int modemId = 0;
    int32_t slotId = StateUtils::GetSlotIdByModemId(modemId);
    EXPECT_EQ(slotId, DEFAULT_SLOT);
}

HWTEST_F(StateUtilsTest, GetSlotIdByModemId002, testing::ext::TestSize.Level1)
{
    int modemId = 1;
    int32_t slotId = StateUtils::GetSlotIdByModemId(modemId);
    EXPECT_NE(slotId, DEFAULT_SLOT);
}

HWTEST_F(StateUtilsTest, ConvertIntSetToString001, testing::ext::TestSize.Level1)
{
    std::set<int> intList = {1, 2, 3};
    std::string str = ConvertIntSetToString(intList);
    EXPECT_EQ(str, "123");
}

HWTEST_F(StateUtilsTest, GetSha256Str001, testing::ext::TestSize.Level1)
{
    std::string str = GetSha256Str("123");
    EXPECT_NE(str, "123");
}

}
}
